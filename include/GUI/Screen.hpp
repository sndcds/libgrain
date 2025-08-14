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
        int32_t m_width = 0;
        int32_t m_height = 0;
        int32_t m_visible_width = 0;
        int32_t m_visible_height = 0;
        void* _m_ns_screen = nullptr;    ///< Pointer to macOS pointer NSScreen

    public:
        Screen() : Object() {}
        virtual ~Screen() {};

        [[nodiscard]] const char* className() const noexcept override { return "Screen"; }

        friend std::ostream& operator << (std::ostream& os, const Screen* o) {
            if (o == nullptr) {
                os << "Screen nullptr";
            }
            else {
                os << "Screen " << o->m_width << " x " << o->m_height << " pixel";
            }
            return os;
        }

        [[nodiscard]] int32_t width() const noexcept { return m_width; }
        [[nodiscard]] int32_t height() const noexcept { return m_height; }
        [[nodiscard]] int32_t visibleWidth() const noexcept { return m_visible_width; }
        [[nodiscard]] int32_t visibleHeight() const noexcept { return m_visible_height; }
        [[nodiscard]] int32_t pixelCount() const noexcept { return m_width * m_height; }
        [[nodiscard]] Rectd rect() const noexcept { return Rectd(0, 0, m_width, m_height); }
        [[nodiscard]] Rectd visibleRect() const noexcept { return Rectd(0, 0, m_visible_width, m_visible_height); }

        [[nodiscard]] void* nsScreen() const noexcept { return _m_ns_screen; }

        [[nodiscard]] bool isValid() const noexcept {
            return m_width > 1 && m_height > 1 && m_visible_width > 1 && m_visible_height > 1;
        }
    };


} // End of namespace Grain

#endif //GrainScreen_hpp
