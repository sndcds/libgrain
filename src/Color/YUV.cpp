//
//  YUV.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//
//  GrYUV.cpp
//  GrainLib
//
//  Created by Roald Christesen on 25.01.24.
//

#include "Color/YUV.hpp"
#include "Color/RGB.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    const Mat3f YUV::g_to_rgb_matrix_601(1.0f, 0.0f, 1.4020f, 1.0f, -0.3441f, -0.7141f, 1.0f, 1.7720f, 0.0f);
    const Mat3f YUV::g_to_rgb_matrix_709(1.0f, 0.0f, 1.5748f, 1.0f, -0.1873f, -0.4681f, 1.0f, 1.8556f, 0.0f);
    const Mat3f YUV::g_from_rgb_matrix_601(0.29899f, 0.58702f, 0.11399f, -0.16873f, -0.33127f, 0.50001f, 0.50001f, -0.41870f, -0.08131f);
    const Mat3f YUV::g_from_rgb_matrix_709(0.212593f, 0.715215f, 0.072192f, -0.114569f, -0.385436f, 0.500004f, 0.500004f, -0.454162f, -0.045842f);


    YUV::YUV(const RGB& rgb, Color::Space color_space) noexcept {

        switch (color_space) {
            case Color::Space::Rec601:
                setRGB601(rgb);
                break;

            case Color::Space::Rec709:
            default:
                setRGB709(rgb);
                break;
        }
    }


    void YUV::setRGB601(const RGB& rgb) noexcept {

        Color::rgb_to_yuv601(rgb.data_, m_data);
    }


    void YUV::setRGB709(const RGB& rgb) noexcept  {

        Color::rgb_to_yuv709(rgb.data_, m_data);
    }


} // End of namespace Grain.
