//
//  Color.hpp
//
//  Created by Roald Christesen on from 28.06.2012
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainColor_hpp
#define GrainColor_hpp

#include "Grain.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    /**
     *  @class Color
     *  @brief Color functionality.
     *
     *  This class contains static functions for working with colors of different types.
     *  It has functions for converting between different color systems like RGB, HSV, YUV and others.
     */
    class Color {
    public:
        static constexpr int32_t kKelvinMin = 1000;
        static constexpr int32_t kKelvinMax = 15000;

        /**
         *  @brief Color model.
         */
        enum class Model {
            Undefined = -1,
            Lumina = 0,
            LuminaAlpha,
            RGB,
            RGBA,
            CMYK,
            YUV,
            XYZ,
            HSV,
            L_a_b,
            Bayer,

            TwoChannel = LuminaAlpha
        };


        /**
         *  @brief Color space.
         */
        enum class Space {
            Undefined = 0,
            sRGB,
            AdobeRGB_1998,
            CIE,
            Apple,
            ProPhoto,
            Rec601,     ///< SD Video
            Rec709,     ///< HD Video
        };


        /**
         *  @brief Color calibration reference.
         */
        enum class CalibrationReference {
            Undefined = 0,
            D50,
            D65
        };


        /**
         *  @brief Color combine modes.
         */
        enum class CombineMode {
            Normal,
            Add,
            Subtract,
            Multiply,
            Screen,
            Overlay,
            SoftLight,
            HardLight,
            Hue,
            Color,
            Luminosity
        };


        /**
         *  @brief Color skin type.
         *
         *  Useful in color grading applications.
         */
        enum class SkinType {
            // TODO: Complete list
            European = 0,
            Indian,
            African
        };


        /**
         *  @brief Gretag Macbeth colors.
         */
        enum class GretagMacbethColor {
            DarkSkin = 0,
            LightSkin,
            BlueSky,
            Foliage,
            BlueFlower,
            BluishGreen,
            Orange,
            PurpischBlue,
            ModerateRed,
            Purple,
            YellowGreen,
            OrangeYellow,
            Blue,
            Green,
            Red,
            Yellow,
            Magenta,
            Cyan,
            White95,
            Neutral80,
            Neutral65,
            Neutral50,
            Neutral35,
            Black20
        };


        /**
         *  @brief Crayola colors.
         */
        enum class CrayolaColor {
            Red = 0,
            RedOrange,
            Orange,
            Yellow,
            YellowGreen,
            Green,
            SkyBlue,
            Blue,
            Violet,
            White,
            Brown,
            Black,
            AquaGreen,
            GoldenYellow,
            Gray,
            JadeGreen,
            LightBlue,
            Magenta,
            Mahogany,
            Peach,
            Pink,
            Tan,
            LightBrown,
            YellowOrange,
            BronzeYellow,
            CoolGray,
            DarkBrown,
            GreenBlue,
            LemonYellow,
            LightOrange,
            Maroon,
            PineGreen,
            Raspberry,
            Salmon,
            Slate,
            Turquoise,
            BubbleGum,
            Cerulean,
            Gold,
            HarvestGold,
            LimeGreen,
            Mango,
            Mauve,
            NavyBlue,
            Orchid,
            PaleRose,
            Sand,
            Silver,
            Taupe,
            Teal,
            Amethyst,
            AuroMetalSaurus,
            BabyBlue,
            BallBlue,
            DollarBill,
            ElectricGreen,
            GuppieGreen,
            MeatBrown,
            Latinum,
            RoseRed,
            Sandstorm,
            SpiroDiscoBall,
            Toolbox,
            UFOGreen,

            Last = UFOGreen
        };


        /**
         *  @brief White balance colors.
         */
        enum class WBColor {
            Candle = 0,
            Tungsten40W,
            Tungsten100W,
            Halogen,
            CarbonArc,
            HighNiinSun,
            DirectSunlight,
            OvercastSky,
            ClearBlueSky,
            WarmFluorescent,
            StandardFluorescent,
            WhiteFluorescent,
            FullSpectrumFluorescent,
            GrowLightFluorescent,
            MercuryVapor,
            SodiumVapor,
            MetalHalide,
            HightPressureSodium,

            Last = HightPressureSodium
        };

        // Constants
        static constexpr float kVectorscopeRedAngle = 12.905752f;
        static constexpr float kLumina709ScaleR = 0.212593f;
        static constexpr float kLumina709ScaleG = 0.715215f;
        static constexpr float kLumina709ScaleB = 0.072192f;
        static constexpr float kLumina601ScaleR = 0.29899f;
        static constexpr float kLumina601ScaleG = 0.58702f;
        static constexpr float kLumina601ScaleB = 0.11399f;

    public:
        static int32_t modelComponentsPerPixel(Model colorModel) noexcept {
            switch (colorModel) {
                case Model::Bayer:
                case Model::Lumina:
                    return 1;
                case Model::LuminaAlpha:
                    return 2;
                case Model::RGB:
                case Model::YUV:
                case Model::XYZ:
                case Model::HSV:
                case Model::L_a_b:
                    return 3;
                case Model::RGBA:
                case Model::CMYK:
                    return 4;
                default:
                    return 0;
            }
        }

        static const char* modelName(Model colorModel) noexcept {
            switch (colorModel) {
                case Model::Lumina: return "Lumina";
                case Model::LuminaAlpha: return "Lumina Alpha";
                case Model::RGB: return "RGB";
                case Model::RGBA: return "RGBA";
                case Model::CMYK: return "CMYK";
                case Model::YUV: return "YUV";
                case Model::XYZ: return "XYZFile";
                case Model::HSV: return "HSV";
                case Model::L_a_b: return "L*a*b";
                case Model::Bayer: return "Bayer Pattern";
                default: return "Undefined";
            }
        }

        static void rgb_to_hsv(const float* rgb, float* out_hsv) noexcept;
        static void rgb_to_hsl(const float* rgb, float* out_hsl) noexcept;
        static void rgb_to_yuv601(const float* rgb, float* out_yuv) noexcept;
        static void rgb_to_yuv709(const float* rgb, float* out_yuv) noexcept;
        static void rgb_to_lab(const float* rgb, float* out_lab) noexcept;
        static void rgb_to_oklab(const float* rgb, float* out_oklab) noexcept;

        static void hsv_to_rgb(const float* hsv, float* out_rgb) noexcept;
        static void hsv_to_hsl(const float* hsv, float* out_hsl) noexcept;

        static void hsl_to_rgb(const float* hsl, float* out_rgb) noexcept;
        static void hsl_to_hsv(const float* hsl, float* out_hsv) noexcept;

        static void yuv601_to_rgb(const float* yuv, float* out_rgb) noexcept;
        static void yuv709_to_rgb(const float* yuv, float* out_rgb) noexcept;

        static void lab_to_rgb(const float* lab, float* out_rgb) noexcept;

        static void oklab_to_rgb(const float* oklab, float* out_rgb) noexcept;
        static void oklab_to_oklch(const float* oklab, float* out_oklch) noexcept;

        static void oklch_to_rgb(const float* oklch, float* out_rgb) noexcept;
        static void oklch_to_oklab(const float* oklch, float* out_oklab) noexcept;

        static void xyz_to_rgb(const float* xyz, const Mat3f& matrix, float* out_rgb) noexcept;
        static void xyz_to_rgb(const float* xyz, const float* m, float* out_rgb) noexcept;

        inline static float gamma_to_linear(float value) noexcept {
            return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
        }
        inline static float linear_to_gamma(float value) noexcept {
            return value < 0.0031308f ? 12.92f*  value : 1.055f*  std::pow(value, 1.0f / 2.4f) - 0.055f;
        }

        static float sony_SLog2_to_linear(float v) noexcept;
        static float sony_Linear_to_SLog2(float v) noexcept;
        static float sony_SLog3_to_Linear(float v) noexcept;
        static float sony_Linear_to_SLog3(float v) noexcept;

        static float combineOverlay(float b, float f) noexcept;
        static float combineScreen(float b, float f) noexcept;
        static float combineSoftLight(float b, float f) noexcept;
        static float combineHardLight(float b, float f) noexcept;
        static float combineLuminance(float r, float g, float b, float l) noexcept;
    };


} // End of namespace Grain

#endif // GrainColor_hpp
