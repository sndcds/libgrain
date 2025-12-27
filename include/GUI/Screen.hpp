//
//  Screen.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainScreen_hpp
#define GrainScreen_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "2d/Rect.hpp"


namespace Grain {

    class App;


    class Screen : public Object {
        friend class App;

    public:
        int32_t width_ = 0;
        int32_t height_ = 0;
        int32_t visible_width_ = 0;
        int32_t visible_height_ = 0;
        void* ns_screen_ = nullptr;    ///< Pointer to macOS pointer NSScreen

    public:
        Screen() : Object() {}
        virtual ~Screen() {};

        [[nodiscard]] const char* className() const noexcept override { return "Screen"; }

        friend std::ostream& operator << (std::ostream& os, const Screen* o) {
            if (!o) {
                os << "Screen nullptr";
            }
            else {
                os << "Screen " << o->width_ << " x " << o->height_ << " pixel";
            }
            return os;
        }

        [[nodiscard]] int32_t width() const noexcept { return width_; }
        [[nodiscard]] int32_t height() const noexcept { return height_; }
        [[nodiscard]] int32_t visibleWidth() const noexcept { return visible_width_; }
        [[nodiscard]] int32_t visibleHeight() const noexcept { return visible_height_; }
        [[nodiscard]] int32_t pixelCount() const noexcept { return width_ * height_; }
        [[nodiscard]] Rectd rect() const noexcept { return Rectd(0, 0, width_, height_); }
        [[nodiscard]] Rectd visibleRect() const noexcept { return Rectd(0, 0, visible_width_, visible_height_); }

        [[nodiscard]] void* nsScreen() const noexcept { return ns_screen_; }

        [[nodiscard]] bool isValid() const noexcept {
            return width_ > 1 && height_ > 1 && visible_width_ > 1 && visible_height_ > 1;
        }
    };


} // End of namespace Grain

#endif //GrainScreen_hpp
