//
//  AnimationFrameDriver.hpp
//

#ifndef GrainAnimationFrameDriver_hpp
#define GrainAnimationFrameDriver_hpp

#include "GUI/Components/Component.hpp"
#include "GUI/macos_ui_helper.h"
#include <chrono>
#include <memory>
#include <functional>
#include <CoreVideo/CoreVideo.h>
#include <iostream>

#include "GUI/Views/View.hpp"


namespace Grain {


using Clock = std::chrono::steady_clock;


class AnimationObject {
public:
    enum class Mode { Finite, Continuous };

    AnimationObject(
        double start,
        double duration,
        std::function<void(AnimationObject*)> callback,
        void* ob)
        : cb_(std::move(callback)), ob_(ob)
    {
        mode_ = duration > 0.0 ? Mode::Finite : Mode::Continuous;
        start_time_ = start <= 0.0 ?
            Clock::now()
            : Clock::now() + std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double>(start));
        duration_ = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(duration));
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
        } else {
            progress_ = std::chrono::duration<double>(now - start_time_).count();
        }

        // Update component state based on progress
        if (auto* c = component()) {
            c->setAnimationProgress(progress_);
            c->needsDisplay();  // ensure redraw
        }

        // Call user callback (can modify position, text, etc.)
        cb_(this);
    }

    void start() {
        start_time_ = Clock::now();
        frame_number_ = 0;
    }

    bool hasStarted() const { return Clock::now() >= start_time_; }
    bool isFinished() const { return mode_ == Mode::Finite && progress_ >= 1.0; }

    Component* component() { return static_cast<Component*>(ob_); }
    uint64_t frameNumber() const noexcept { return frame_number_; }

    AnimationObject* next_ = nullptr;
    double progress_ = 0.0;

private:
    /* TODO: remove
    static void runComponentUpdate(void* context) {
        ComponentUpdateData* data = static_cast<ComponentUpdateData*>(context);
        if (data && data->comp) {
            data->comp->setAnimationProgress(data->progress);
            data->comp->needsDisplay();
        }
        delete data; // clean up
    }

    void dispatchComponentUpdate() {
        if (!ob_) return;
        auto* data = new ComponentUpdateData{ static_cast<Component*>(ob_), progress_ };
        GrainUIHelper_runOnMainThread(runComponentUpdate, data);
    }
    */

    struct ComponentUpdateData {
        Component* comp;
        double progress;
    };

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
        std::function<void(AnimationObject*)> callback,
        void* ob)
    {
        if (!callback || !ob) return nullptr;
        auto ao = std::make_unique<AnimationObject>(start, duration, callback, ob);
        ao->start();
        ao->next_ = first_;
        first_ = ao.release();
        return first_;
    }

    void removeAnimation(AnimationObject* animation_object) {
        if (!animation_object) return;

        if (animation_object == first_) {
            first_ = first_->next_;
        } else {
            AnimationObject* prev = first_;
            while (prev && prev->next_ != animation_object) prev = prev->next_;
            if (prev) prev->next_ = animation_object->next_;
        }
        delete animation_object;
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
        if (displayLink_) return; // already started

        // Create CVDisplayLink to sync to screen refresh
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink_);
        CVDisplayLinkSetOutputCallback(displayLink_, &AnimationFrameDriver::displayLinkCallback, this);
        CVDisplayLinkStart(displayLink_);
    }

private:
    AnimationFrameDriver() = default;
    ~AnimationFrameDriver() {
        if (displayLink_) {
            CVDisplayLinkStop(displayLink_);
            CVDisplayLinkRelease(displayLink_);
        }
    }

    AnimationObject* first_ = nullptr;
    CVDisplayLinkRef displayLink_ = nullptr;

    static CVReturn displayLinkCallback(
        CVDisplayLinkRef,
        const CVTimeStamp*,
        const CVTimeStamp*,
        CVOptionFlags,
        CVOptionFlags*,
        void* context)
    {
        auto* driver = static_cast<AnimationFrameDriver*>(context);

        GrainUIHelper_runOnMainThread(runDriverTick, driver);

        return kCVReturnSuccess;
    }

    static void runDriverTick(void* context) {
        auto* driver = static_cast<AnimationFrameDriver*>(context);
        if (driver) {
            driver->tick();
        }
    }
};

} // namespace Grain

#endif // GrainAnimationFrameDriver_hpp