//
//  AnimationFrameDriver.hpp
//
//  Created by Roald Christesen on from 30.12.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainAnimationFrameDriver_hpp
#define GrainAnimationFrameDriver_hpp

#include "GUI/Components/Component.hpp"

#include <chrono>
#include <memory>
#include <functional>
#include <dispatch/dispatch.h>


namespace Grain {

using Clock = std::chrono::steady_clock;

class AnimationObject {
public:
    enum class Mode { Finite, Continuous };

    AnimationObject(double start, double duration, std::function<void(AnimationObject*)> callback, void* ob)
        : cb_(std::move(callback)), ob_(ob)
    {
        mode_ = duration > 0.0 ? Mode::Finite : Mode::Continuous;

        start_time_ = start <= 0.0 ?
            Clock::now()
            : Clock::now() + std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(start));
        duration_ = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(duration));
        progress_ = 0.0;
        next_ = nullptr;
    }

    void updateProgress() {
        if (!ob_ || !cb_) return;

        auto now = Clock::now();
        if (now < start_time_) return;

        ++frame_number_;

        if (mode_ == Mode::Finite) {
            auto elapsed = std::chrono::duration<double>(now - start_time_).count();
            progress_ = elapsed / std::chrono::duration<double>(duration_).count();
            if (progress_ > 1.0) progress_ = 1.0;
        }
        else {
            progress_ = std::chrono::duration<double>(now - start_time_).count();
        }

        dispatchComponentUpdate();
        if (cb_) {
            cb_(this);
        }
    }

    void start() {
        start_time_ = Clock::now();
        frame_number_ = 0;
    }

    bool hasStarted() const {
        return Clock::now() >= start_time_;
    }

    bool isFinished() const {
        return mode_ == Mode::Finite && progress_ >= 1.0;
    }

    Component* component() { return static_cast<Component*>(ob_); }
    uint64_t frameNumber() const noexcept { return frame_number_; }

    AnimationObject* next_ = nullptr;
    double progress_ = 0.0;

private:
    void dispatchComponentUpdate() {
#if defined(__APPLE__) && defined(__MACH__)
        if (!ob_) return;

        struct Context {
            Component* component;
            double progress;
        };

        auto ctx = new Context{ static_cast<Component*>(ob_), progress_ };

        // Dispatch asynchronously on main thread safely
        dispatch_async_f(dispatch_get_main_queue(), ctx, [](void* userData){
            std::unique_ptr<Context> ctxPtr(static_cast<Context*>(userData));
            if (ctxPtr->component) {
                ctxPtr->component->setAnimationProgress(ctxPtr->progress);
                ctxPtr->component->needsDisplay();
            }
        });
#endif
    }

    Mode mode_;
    Clock::time_point start_time_;
    Clock::duration duration_;
    uint64_t frame_number_ = 0;
    std::function<void(AnimationObject*)> cb_;
    void* ob_;
};

class AnimationFrameDriver {
public:
    static AnimationFrameDriver& instance() {
        static AnimationFrameDriver driver;
        return driver;
    }

    AnimationObject* animate(
        double start,
        double duration,
        std::function<void(AnimationObject*)> callback, void* ob)
    {
        if (!callback || !ob) return nullptr;

        auto ao = std::make_unique<AnimationObject>(start, duration, callback, ob);
        ao->next_ = first_;
        first_ = ao.release();
        std::cout << "animate() start: " << start << ", duration: " << duration << std::endl;
        return first_;
    }

    void removeAnimation(AnimationObject* animation_object) {
        if (!animation_object) return;

        if (animation_object == first_) {
            first_ = first_->next_;
        }
        else {
            AnimationObject* prev = first_;
            while (prev && prev->next_ != animation_object) prev = prev->next_;
            if (prev) prev->next_ = animation_object->next_;
        }
        delete animation_object;
        std::cout << "removed\n";
    }

    void tick() {
        AnimationObject* ao = first_;
        while (ao) {
            AnimationObject* next = ao->next_;
            ao->updateProgress();
            if (ao->isFinished()) {
                removeAnimation(ao);
            }
            ao = next;
        }
    }

    void start() {
        if (timer_) return; // already started

        timer_ = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        if (!timer_) return;

        dispatch_source_set_timer(timer_,
                                  DISPATCH_TIME_NOW,
                                  16 * NSEC_PER_MSEC,
                                  1 * NSEC_PER_MSEC);

        // Pass `this` as the context
        dispatch_source_set_event_handler_f(timer_, &AnimationFrameDriver::timerCallback);
        dispatch_set_context(timer_, this);

        dispatch_resume(timer_);
    }

private:
    AnimationFrameDriver() = default;
    AnimationObject* first_ = nullptr;
    dispatch_source_t timer_ = nullptr;

    static void timerCallback(void* context) {
        auto* driver = static_cast<AnimationFrameDriver*>(context);
        if (driver) driver->tick();
    }
};

} // namespace Grain

#endif // GrainAnimationFrameDriver_hpp