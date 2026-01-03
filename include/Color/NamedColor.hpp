//
//  NamedColor.hpp
//
//  Created by Roald Christesen on from 18.04.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainNamedColor_hpp
#define GrainNamedColor_hpp

#include "Color/RGB.hpp"
#include "Color/Color.hpp"
#include "String/String.hpp"


namespace Grain {


    class NamedColor : public RGB {

    public:
        enum {
            kGretagMacbethColorCount = 24,
            kCrayolaColorCount = 64
        };

        static const NamedColor g_gretag_macbeth_colors[kGretagMacbethColorCount];
        static const NamedColor g_crayola_colors[kCrayolaColorCount];

    private:
        String m_name = "Unnamed";

    public:
        NamedColor(const String& name, float r, float g, float b) noexcept : RGB(r, g, b) {
            m_name = name;
        }

        NamedColor(const String& name, uint32_t value) noexcept {
            m_name = name;
            set24bit(value);
        }

        ~NamedColor() noexcept {}



        String name() const noexcept { return m_name; }

        static const NamedColor gretagMacbethColor(Color::GretagMacbethColor index) noexcept {
            return NamedColor::g_gretag_macbeth_colors[std::clamp<int32_t>(static_cast<int32_t>(index), 0, kGretagMacbethColorCount - 1)];
        }

        static const float* gretagMacbethColorDataPtr(Color::GretagMacbethColor index) noexcept {
            return NamedColor::g_gretag_macbeth_colors[std::clamp<int32_t>(static_cast<int32_t>(index), 0, kGretagMacbethColorCount - 1)].data_;
        }

        static const NamedColor crayolaColor(Color::CrayolaColor index) noexcept {
            return NamedColor::g_crayola_colors[std::clamp<int32_t>(static_cast<int32_t>(index), 0, kCrayolaColorCount - 1)];
        }

        static const float* crayolaColorDataPtr(Color::CrayolaColor index) noexcept {
            return NamedColor::g_crayola_colors[std::clamp<int32_t>(static_cast<int32_t>(index), 0, kCrayolaColorCount - 1)].data_;
        }
    };


} // End of namespace Grain

#endif // GrainNamedColor_hpp
