//
//  AnimationDriver.cpp
//
//  Created by Roald Christesen on from 27.12.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include <Graphic/AnimationFrameDriver.hpp>
#include <GUI/Components/Component.hpp>

namespace Grain {

    AnimationObject::AnimationObject(double start, double duration, AnimationCallback callback, void* ob, void* ref) noexcept {
        mode_ = AnimationMode::Finite;

        if (start <= 0.0) {
            start_ = Clock::now();
        } else {
            auto start_casted = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(start));
            start_ = Clock::time_point(start_casted);
        }

        duration_ = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(duration));

        cb_ = callback;
        ob_ = ob;
        ref_ = ref;
    }


    void AnimationObject::componentNeedsDisplay() {
        auto component = static_cast<Component*>(object());
        if (!component) return;

        component->setAnimationProgress(progress_);

#if defined(__APPLE__) && defined(__MACH__)
        dispatch_async_f(
                dispatch_get_main_queue(),
                component,
                [](void* ctx) {
                    static_cast<Component*>(ctx)->needsDisplay();
                }
            );
#else
        component->needsDisplay();
#endif
    }

    static AnimationFrameDriver& instance() {
        static AnimationFrameDriver driver;
        return driver;
    }

    void AnimationFrameDriver::start() {
#if defined(__APPLE__) && defined(__MACH__)
        if (!link_) {
            CVDisplayLinkCreateWithActiveCGDisplays(&link_);
            CVDisplayLinkSetOutputCallback(link_, &AnimationFrameDriver::displayLinkCallback, this);
        }
        if (!CVDisplayLinkIsRunning(link_)) {
            CVDisplayLinkStart(link_);
        }
#endif
    }

    void AnimationFrameDriver::stop() {
#if defined(__APPLE__) && defined(__MACH__)
        if (link_ && CVDisplayLinkIsRunning(link_)) {
            CVDisplayLinkStop(link_);
        }
#endif
    }

    AnimationObject* AnimationFrameDriver::animate(
        double start,
        double duration,
        AnimationCallback callback,
        void* ob,
        void* ref
    )
    {
        if (!callback || !ob || duration < 1.0 / 60.0) { return nullptr; }
        auto ao = new(std::nothrow) AnimationObject(start, duration, callback, ob, ref);
        if (ao) {
            ao->next_ = first_ob_;
            first_ob_ = ao;
        }
        return ao;
    }

    void AnimationFrameDriver::removeAnimation(AnimationObject* animation_object) {
        if (!animation_object) { return; }

        if (animation_object == first_ob_) {
            first_ob_ = animation_object->next_;
        }
        else {
            AnimationObject* ao = first_ob_;
            while (ao) {
                if (ao->next_ == animation_object) {
                    ao->next_ = animation_object->next_;
                }
                ao = ao->next_;
            }

        }
        delete animation_object;
    }


    void AnimationFrameDriver::tick() {
        auto now = std::chrono::steady_clock::now();
        auto ao = first_ob_;
        while (ao) {
            auto next = ao->next_;
            if (ao->callback()) {
                auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - ao->start_);
                double progress = elapsed.count() / std::chrono::duration<double>(ao->duration_).count();
                ao->progress_ = progress > 1.0 ? 1.0 : progress;
                ao->cb_(ao);
            }

            // optionally remove finished animations
            if (ao->progress_ >= 1.0 && ao->mode_ == AnimationObject::AnimationMode::Finite) {
                removeAnimation(ao);
            }

            ao = next;
        }
    }
} // End of namespace Grain
