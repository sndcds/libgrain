//
//  AnimationDriver.hpp
//
//  Created by Roald Christesen on from 27.12.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAnimationDriver_hpp
#define GrainAnimationDriver_hpp

#include <Type/List.hpp>
#include <Time/Timestamp.hpp>

#include <chrono>
#include <functional>


#if defined(__APPLE__) && defined(__MACH__)
#include <CoreVideo/CoreVideo.h>
#endif

namespace Grain {

class AnimationObject;

    typedef void (*AnimationCallback)(AnimationObject* animation_object);

class AnimationObject {

    friend class AnimationFrameDriver;

public:
    enum class AnimationMode {
        None,
        Finite,
        Continuous
    };

protected:
    AnimationMode mode_ = AnimationMode::None;
    TimePoint start_{};
    Duration duration_{};
    double progress_{};
    AnimationCallback cb_ = nullptr;
    void* ob_ = nullptr;
    void* ref_ = nullptr;
    AnimationObject* next_ = nullptr;

public:
    AnimationObject(double start, double duration, AnimationCallback callback, void* ob, void* ref) noexcept;
    void componentNeedsDisplay();

    [[nodiscard]] AnimationCallback callback() const noexcept { return cb_; }
    [[nodiscard]] void* object() const noexcept { return ob_; }
    [[nodiscard]] double progress() const noexcept { return progress_; }
};


class AnimationFrameDriver {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Callback = std::function<void(TimePoint)>;


    static AnimationFrameDriver& instance() {
        static AnimationFrameDriver driver;
        return driver;
    }

    void start();
    void stop();

    AnimationObject* animate(double start, double duration, AnimationCallback callback, void* ob, void* ref);
    void removeAnimation(AnimationObject* animation_object);

private:
    AnimationObject* first_ob_ = nullptr;


#if defined(__APPLE__) && defined(__MACH__)
    CVDisplayLinkRef link_{nullptr};

    static CVReturn displayLinkCallback(
        CVDisplayLinkRef,
        const CVTimeStamp*, const CVTimeStamp*,
        CVOptionFlags, CVOptionFlags*, void* userData)
    {
        auto* driver = static_cast<AnimationFrameDriver*>(userData);
        driver->tick();
        return kCVReturnSuccess;
    }
#endif

    void tick();
};

} // End of namespace Grain

#endif // GrainAppleCGContext_hpp