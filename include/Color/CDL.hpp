//
//  CDL.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainCDL_hpp
#define GrainCDL_hpp

#include "Grain.hpp"
#include "Color/RGB.hpp"
#include "Color/HSV.hpp"


namespace Grain {

    /**
     *  @brief Color Decision List (CDL).
     *
     *  A Color Decision List (CDL) is a standardized method for communicating color adjustments and corrections in video editing and color grading workflows.
     *  CDLs encapsulate sets of instructions that define color transformations, enhancements, and corrections to be applied to video footage.
     *  They include parameters such as lift, gamma, and gain that control the shadows, midtones, and highlights of the image.
     *
     *  CDLs enable consistent color grading across different software applications and systems involved in post-production.
     *  They provide a reliable way to transfer and apply color adjustments between platforms, ensuring that the intended visual style is maintained.
     *  In the film and broadcast industries, CDLs facilitate collaboration and accurate color reproduction from one stage of production to another.
     *
     *  By using CDLs, colorists and editors can efficiently achieve the desired color look while maintaining uniformity and accuracy in color grading workflows.
     */
    class CDL_RGB {

    public:
        RGB m_lift_rgb;
        RGB m_gamma_rgb;
        RGB m_gain_rgb;
        RGB m_shift1_rgb;
        RGB m_shift2_rgb;
    };

    class CDL {
    public:
        float m_black_point = 0.063f;
        float m_lift = 0.0f;
        float m_gamma = 2.0f;
        float m_gain = 2.0f;
        HSV m_lift_hsv = { 0.0f, 0.0f, 0.0f };
        HSV m_gamma_hsv = { 0.0f, 0.0f, 0.5f };
        HSV m_gain_hsv = { 0.0f, 0.0f, 0.5f };

    public:
        CDL() noexcept {}
        ~CDL() noexcept {}

        void init() noexcept;
        void buildCDL_RGB(CDL_RGB& outCDL_RGB) const noexcept;
    };


} // End of namespace Grain

#endif // GrainCDL_hpp
