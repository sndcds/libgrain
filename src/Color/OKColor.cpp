//
//  OKColor.hpp
//
//  Created by Roald Christesen on from 19.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/OKColor.hpp"
#include "Color/RGB.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    OKLab::OKLab(const OKLCh& oklch) noexcept {

        Color::oklch_to_oklab(oklch.m_data, m_data);
    }


    OKLab::OKLab(const RGB& rgb) noexcept {

        Color::rgb_to_oklab(rgb.m_data, m_data);
    }


    OKLCh::OKLCh(const RGB& rgb) noexcept {

        OKLab oklab;
        Color::rgb_to_oklab(rgb.m_data, oklab.m_data);
        Color::oklab_to_oklch(oklab.m_data, m_data);
    }

    OKLCh::OKLCh(const OKLab& oklab) noexcept {

        Color::oklab_to_oklch(oklab.m_data, m_data);
    }

} // End of namespace Grain
