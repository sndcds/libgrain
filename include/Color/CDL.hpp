//
//  CDL.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
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
        RGB lift_rgb_;
        RGB gamma_rgb_;
        RGB gain_rgb_;
        RGB shift1_rgb_;
        RGB shift2_rgb_;
    };

    class CDL {
    public:
        float black_point_ = 0.063f;
        float lift_ = 0.0f;
        float gamma_ = 2.0f;
        float gain_ = 2.0f;
        HSV lift_hsv_ = { 0.0f, 0.0f, 0.0f };
        HSV gamma_hsv_ = { 0.0f, 0.0f, 0.5f };
        HSV gain_hsv_ = { 0.0f, 0.0f, 0.5f };

    public:
        CDL() noexcept {}
        ~CDL() noexcept {}

        void init() noexcept;
        void buildCDL_RGB(CDL_RGB& out_cdl_rgb) const noexcept;
    };


} // End of namespace Grain

#endif // GrainCDL_hpp
