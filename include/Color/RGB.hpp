//
//  RGB.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainRGB_hpp
#define GrainRGB_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"
#include "Type/Type.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {

    class RGB;
    class HSV;
    class HSL;
    class YUV;
    class CIEXYZ;
    class CIExyY;
    class OKLab;
    class OKLCh;
    class LMS;
    class CDL;
    class CDL_RGB;
    class LUT1;
    class String;


    /**
     *  @typedef RGBCombineFunc
     *  @brief Function type for operations combining two RGB colors.
     *
     *  Defines the function type for functions that combine an RGB color `a`
     *  with another RGB color `b`. The combination modifies the color `a`
     *  based on the logic implemented in the function.
     *
     *  @param[out] a The RGB color to be modified by the combination.
     *  @param[in]  b The RGB color used in the combination with `a`.
     *
     *  @note This typedef can be used to define function pointers for
     *        operations such as blending, adding, or subtracting two
     *        RGB colors.
     *
     *  @see RGB
     */
    typedef void (*RGBCombineFunc)(RGB& a, const RGB& b);


    /**
     *  @class RGB
     *  @brief RGB Color Description.
     *
     *  Represents a color in the RGB color space, defined by its red, green, and blue components. Each component has
     *  a value within the range of 0 to 1, representing the intensity of that specific color channel. By combining
     *  varying intensities of red, green, and blue, a wide spectrum of colors can be represented.
     *
     *  This class can be used to represent colors in multiple RGB color spaces, including:
     *  - **sRGB**: The standard RGB color space commonly used for displays and the web.
     *  - **Linear RGB**: A linearized version of RGB, where color values are proportional to light intensity.
     *  - **Custom RGB Spaces**: Can be extended for other RGB representations as required.
     *
     *  @note The `RGB` class does not inherently account for lightness or perceived brightness of the color.
     *  These properties are dependent on the specific RGB color space in use (e.g., sRGB or Linear RGB)
     *  and must be calculated separately using appropriate formulas. This allows the `RGB` class to remain
     *  versatile and adaptable across different RGB spaces.
     */
    class RGB {

    public:
        static const RGB kBlack;    ///< Default color black
        static const RGB kWhite;    ///< Default color white
        static const RGB kRed;
        static const RGB kGreen;
        static const RGB kBlue;
        static const RGB kCyan;
        static const RGB kMagenta;
        static const RGB kYellow;
        static const RGB kMixboxCadmiumYellow;
        static const RGB kMixboxHansaYellow;
        static const RGB kMixboxCadmiumOrange;
        static const RGB kMixboxCadmiumRed;
        static const RGB kMixboxQuinacridoneMagenta;
        static const RGB kMixboxCobaltViolet;
        static const RGB kMixboxUltramarineBlue;
        static const RGB kMixboxCobaltBlue;
        static const RGB kMixboxPhthaloBlue;
        static const RGB kMixboxPhthaloGreen;
        static const RGB kMixboxPermanentGreen;
        static const RGB kMixboxSapGreen;
        static const RGB kMixboxBurntSienna;

    public:
        float data_[3]{};  ///< The RGB color component values

    public:
        RGB() noexcept = default;
        RGB(float r, float g, float b) noexcept;

        explicit RGB(float value) noexcept;
        explicit RGB(int32_t r, int32_t g, int32_t b, int32_t max) noexcept;
        explicit RGB(const float* values) noexcept;
        explicit RGB(const RGB& rgb, float scale) noexcept;
        explicit RGB(const RGB& a, const RGB& b, float t) noexcept;
        explicit RGB(const HSV& hsv) noexcept;
        explicit RGB(const HSL& hsl) noexcept;
        explicit RGB(const YUV& yuv, Color::Space yuv_color_space = Color::Space::Rec709) noexcept;
        explicit RGB(const CIEXYZ& xyz) noexcept;
        explicit RGB(const CIExyY& xyY) noexcept;
        explicit RGB(const OKLab& oklab) noexcept;
        explicit RGB(const OKLCh& oklch) noexcept;
        explicit RGB(const LMS& lms) noexcept;
        explicit RGB(uint32_t value) noexcept;
        explicit RGB(Color::GretagMacbethColor gretag_macbeth_color) noexcept;
        explicit RGB(Color::CrayolaColor crayola_color) noexcept;
        explicit RGB(const String& csv) noexcept;
        explicit RGB(const char* csv) noexcept;

        virtual ~RGB() noexcept = default;


        [[nodiscard]] virtual const char* className() const noexcept { return "RGB"; }

        friend std::ostream& operator << (std::ostream& os, const RGB* o) {
            o == nullptr ? os << "RGB nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RGB& o) {
            os << o.data_[0] << ", " << o.data_[1] << ", " << o.data_[2];
            return os;
        }

        // Operator overloading
        RGB& operator = (uint32_t v) { set24bit(v); return *this; }
        RGB& operator = (float v) { data_[0] = data_[1] = data_[2] = v; return *this; }

        RGB& operator = (const RGB&) = default;
        RGB& operator = (const HSV& v);
        RGB& operator = (const HSL& v);
        RGB& operator = (const OKLCh& v);
        RGB& operator = (const OKLab& v);
        RGB& operator = (Color::GretagMacbethColor v);
        RGB& operator = (Color::CrayolaColor v);

        bool operator == (const RGB& v) const {
            return data_[0] == v.data_[0] && data_[1] == v.data_[1] && data_[2] == v.data_[2];
        }

        bool operator != (const RGB& v) const {
            return data_[0] != v.data_[0] || data_[1] != v.data_[1] || data_[2] != v.data_[2];
        }

        RGB operator + (const RGB& v) const {
            return RGB(data_[0] + v.data_[0], data_[1] + v.data_[1], data_[2] + v.data_[2]);
        }

        RGB operator + (float v) const { return RGB(data_[0] + v, data_[1] + v, data_[2] + v); }

        RGB operator - (const RGB& v) const {
            return RGB(data_[0] - v.data_[0], data_[1] - v.data_[1], data_[2] - v.data_[2]);
        }

        RGB operator - (float v) const { return RGB(data_[0] - v, data_[1] - v, data_[2] - v); }

        RGB operator * (const RGB& v) const {
            return RGB(data_[0] * v.data_[0], data_[1] * v.data_[1], data_[2] * v.data_[2]);
        }

        RGB operator * (float v) const { return RGB(data_[0] * v, data_[1] * v, data_[2] * v); }

        RGB operator / (const RGB& v) const {
            return RGB(data_[0] / v.data_[0], data_[1] / v.data_[1], data_[2] / v.data_[2]);
        }

        RGB operator / (float v) const { return RGB(data_[0] / v, data_[1] / v, data_[2] / v); }

        RGB& operator += (const RGB& v) {
            data_[0] += v.data_[0]; data_[1] += v.data_[1]; data_[2] += v.data_[2];
            return *this;
        }

        RGB& operator += (float v) { data_[0] += v; data_[1] += v; data_[2] += v; return *this; }

        RGB& operator -= (const RGB& v) {
            data_[0] -= v.data_[0]; data_[1] -= v.data_[1]; data_[2] -= v.data_[2];
            return *this;
        }

        RGB& operator -= (float v) { data_[0] -= v; data_[1] -= v; data_[2] -= v; return *this; }

        RGB& operator *= (const RGB& v) {
            data_[0] *= v.data_[0];
            data_[1] *= v.data_[1];
            data_[2] *= v.data_[2];
            return *this;
        }

        RGB& operator *= (float v) { data_[0] *= v; data_[1] *= v; data_[2] *= v; return *this; }

        RGB& operator /= (const RGB& v) {
            data_[0] /= v.data_[0]; data_[1] /= v.data_[1]; data_[2] /= v.data_[2];
            return *this;
        }

        // Get
        [[nodiscard]] virtual bool isValid() noexcept {
            return (data_[0] >= 0.0f && data_[1] >= 0.0f && data_[2] >= 0.0f);
        }
        [[nodiscard]] virtual bool isInvalid() noexcept {
            return (data_[0] < 0.0f || data_[1] < 0.0f || data_[2] < 0.0f);
        }

        [[nodiscard]] const float* valuePtr() noexcept { return data_; }
        [[nodiscard]] float* mutValuePtr() noexcept { return data_; }
        [[nodiscard]] float red() const noexcept { return data_[0]; }
        [[nodiscard]] float green() const noexcept { return data_[1]; }
        [[nodiscard]] float blue() const noexcept { return data_[2]; }
        [[nodiscard]] uint8_t redUInt8() const noexcept { return Type::floatToUInt8(data_[0]); }
        [[nodiscard]] uint8_t greenUInt8() const noexcept { return Type::floatToUInt8(data_[1]); }
        [[nodiscard]] uint8_t blueUInt8() const noexcept { return Type::floatToUInt8(data_[2]); }
        [[nodiscard]] uint32_t rgb24bit() const noexcept;
        virtual void values(float* out_values) const noexcept;

        void hexString(String& out_string, bool upper_case = false, bool c_style = false) const noexcept;

        [[nodiscard]] float lumina(Color::Space yuv_color_space = Color::Space::Rec709) const noexcept;
        [[nodiscard]] float lumina601() const noexcept;
        [[nodiscard]] float lumina709() const noexcept;

        [[nodiscard]] float hsvValue() const noexcept;
        [[nodiscard]] Vec2f CIExy() const noexcept;
        [[nodiscard]] float uvAngle() const noexcept;
        [[nodiscard]] float distance(const RGB& rgb) const noexcept;
        [[nodiscard]] float perceptualDistance(const RGB& rgb) const noexcept;

        [[nodiscard]] bool isDark() const noexcept;
        [[nodiscard]] bool isSame(const RGB& rgb, float tolerance = 0.0001f) const noexcept;

        [[nodiscard]] RGB inverted() const noexcept { return RGB(1.0f - data_[0], 1.0f - data_[1], 1.0f - data_[2]); }

        // Set
        virtual void setInvalid() noexcept { data_[0] = data_[1] = data_[2] = -1; }

        virtual void black() noexcept { data_[0] = data_[1] = data_[2] = 0.0f; }
        virtual void white() noexcept { data_[0] = data_[1] = data_[2] = 1.0f; }

        void set(float r, float g, float b) noexcept { data_[0] = r; data_[1] = g; data_[2] = b; }
        virtual void setGrey(float value) noexcept { data_[0] = data_[1] = data_[2] = value; }
        void setRed(float r) noexcept { data_[0] = r; }
        void setGreen(float g) noexcept { data_[1] = g; }
        void setBlue(float b) noexcept { data_[2] = b; }
        void setValue(float value) noexcept { data_[0] = data_[1] = data_[2] = value; };
        void set24bit(uint32_t value) noexcept;
        void setUInt8(uint8_t r, uint8_t g, uint8_t b) noexcept;
        virtual void setValues(const float* values) noexcept;
        virtual void setValues(const float* values, float scale) noexcept;
        virtual bool setSystemAndValues(const char* system_name, float v1 = 0.0f, float v2 = 0.0f, float v3 = 0.0f, float v4 = 0.0f) noexcept;
        void setIntRGB(int32_t r, int32_t g, int32_t b, int32_t max) noexcept;
        void setHSV(float h, float s, float v) noexcept;
        void setYUV(const YUV& yuv, Color::Space yuv_color_space = Color::Space::Rec709) noexcept;
        void setYUV601(const YUV& yuv) noexcept;
        void setYUV709(const YUV& yuv) noexcept;
        void setXYZ(const CIEXYZ& xyz) noexcept;
        void setXyY(const CIExyY& xyY) noexcept;
        void setCIExy(const Vec2f& xy) noexcept;
        void setOKLab(const OKLab& oklab) noexcept;
        void setOKLCh(const OKLCh& oklch) noexcept;
        void setOKLCh(float l, float c, float h) noexcept;
        void setSkinColor(Color::SkinType skin_type, float value) noexcept;
        void setKelvin(float temperature) noexcept;

        void setLerp(const RGB& a, const RGB& b, double t) noexcept {
            data_[0] = a.data_[0] + t * (b.data_[0] - a.data_[0]);
            data_[1] = a.data_[1] + t * (b.data_[1] - a.data_[1]);
            data_[2] = a.data_[2] + t * (b.data_[2] - a.data_[2]);
        }

        virtual int32_t setByCSV(const char* csv) noexcept;

        void setPosColor(const Vec3d& pos) noexcept;  // TODO: Rename!
        void setByPosOnCircle(float angle, float distance) noexcept;  // TODO: Rename!

        // Randomize
        void random() noexcept;
        void random(float min, float max) noexcept;
        void randomGrey() noexcept;
        void randomGrey(float min, float max) noexcept;
        void randomHSV(float min_h, float max_h, float min_s, float max_s, float min_v, float max_v) noexcept;

        // Modify
        void normalize() noexcept;
        void clamp() noexcept;
        void clamp(float max) noexcept;
        void clamp(float min, float max) noexcept;
        void clamp(const RGB& min, const RGB& max) noexcept;
        void clampMin(const RGB& min) noexcept;
        void clampMax(const RGB& max) noexcept;
        void invert() noexcept;
        void rotateHue(float angle) noexcept;
        virtual void scale(float scale) noexcept;
        void scaleValue(float scale) noexcept;
        void applyCDL(const CDL& cdl) noexcept;
        void applyCDL(const CDL_RGB& cdl_rgb) noexcept;
        void applyRGBLUT(const LUT1& red_lut, const LUT1& green_lut, const LUT1& blue_lut) noexcept;
        void swapRedGreen() noexcept { float temp = data_[0]; data_[0] = data_[1]; data_[1] = temp; }
        void swapRedBlue() noexcept { float temp = data_[0]; data_[0] = data_[2]; data_[2] = temp; }
        void swapGreenBlue() noexcept { float temp = data_[1]; data_[1] = data_[2]; data_[2] = temp; }
        void swap(RGB& color) noexcept { RGB temp = *this; *this = color; color = temp; }

        // Gamma etc.
        void applyPow(float e) noexcept;
        void linearTosRGB() noexcept;
        void sRGBToLinear() noexcept;
        void sonySLog2ToLinear() noexcept;
        void sonySLog3ToLinear() noexcept;
        void sonyLinearToSLog2() noexcept;
        void sonyLinearToSLog3() noexcept;

        // Transform
        void transform(const Mat3f& matrix) noexcept;
        void transform(const Mat3f& matrix, RGB& out_rgb) const noexcept;
        void transform(const Mat3f& matrix, CIEXYZ& out_xyz) const noexcept;

        // Blend
        [[nodiscard]] RGB blend(const RGB& rgb, float t) const noexcept;
        void setBlend(const RGB& other, float t) noexcept;
        void setBlend(const RGB& a, const RGB& b, float t) noexcept;
        void setBlend(const RGB& a, const RGB& b, const RGB& c, float t) noexcept;
        void setBlendWhite(float t) noexcept;
        void setBlendBlack(float t) noexcept;

        void mixbox(const RGB& color1, const RGB& color2, float t) noexcept;
        void mixbox3(const RGB& color1, const RGB& color2, const RGB& color3, float f1, float f2, float f3) noexcept;

        // Combine
        [[nodiscard]] static RGBCombineFunc rgbCombineFunc(Color::CombineMode combine_mode) noexcept;

        static void combineNormal(RGB& a, const RGB& b) noexcept;
        static void combineAdd(RGB& a, const RGB& b) noexcept;
        static void combineSubtract(RGB& a, const RGB& b) noexcept;
        static void combineMultiply(RGB& a, const RGB& b) noexcept;
        static void combineScreen(RGB& a, const RGB& b) noexcept;
        static void combineOverlay(RGB& a, const RGB& b) noexcept;
        static void combineSoftLight(RGB& a, const RGB& b) noexcept;
        static void combineHardLight(RGB& a, const RGB& b) noexcept;
        static void combineHue(RGB& a, const RGB& b) noexcept;
        static void combineColor(RGB& a, const RGB& b) noexcept;
        static void combineLuminosity(RGB& a, const RGB& b) noexcept;

        // Direct to memory conversion
        void readFromMem(const float* ptr) noexcept;
        void writeToMem(float* ptr) const noexcept;
        void writeToMemUInt8(uint8_t* ptr) const noexcept;
        void writeToMemUInt16(uint16_t* ptr) const noexcept;

        // User interface
        [[nodiscard]] bool isValidUiColor() const noexcept { return data_[0] >= 0; }
        [[nodiscard]] RGB uiTextColor(bool enabled) const noexcept;
        static RGB statusColor(bool selected, bool highlighted, const RGB& bg_color, const RGB& fg_color) noexcept;


        #if defined(__APPLE__) && defined(__MACH__)
            [[nodiscard]] CGColorRef createCGColor(float alpha = 1.0f) const noexcept;
        #endif

        // Helper
        inline static void swap(RGB& a, RGB& b) noexcept {
            RGB temp = a;
            a = b;
            b = temp;
        }

        inline static float u8_to_lumina_601(uint8_t r, uint8_t g, uint8_t b) {
            return (Color::kLumina601ScaleR * static_cast<float>(r) +
                    Color::kLumina601ScaleG * static_cast<float>(g) +
                    Color::kLumina601ScaleB * static_cast<float>(g)) / std::numeric_limits<uint8_t>::max();
        }

        inline static float u16_to_lumina_601(uint16_t r, uint16_t g, uint16_t b) {
            return (Color::kLumina601ScaleR * static_cast<float>(r) +
                    Color::kLumina601ScaleG * static_cast<float>(g) +
                    Color::kLumina601ScaleB * static_cast<float>(g)) / std::numeric_limits<uint16_t>::max();
        }

        inline static float float_to_lumina_601(float r, float g, float b) {
            return Color::kLumina601ScaleR * r + Color::kLumina601ScaleG * g + Color::kLumina601ScaleB * g;
        }

        inline static float u8_to_lumina_709(uint8_t r, uint8_t g, uint8_t b) {
            return (Color::kLumina709ScaleR * static_cast<float>(r) +
                    Color::kLumina709ScaleG * static_cast<float>(g) +
                    Color::kLumina709ScaleB * static_cast<float>(g)) / std::numeric_limits<uint8_t>::max();
        }

        inline static float u16_to_lumina_709(uint16_t r, uint16_t g, uint16_t b) {
            return (Color::kLumina709ScaleR * static_cast<float>(r) +
                    Color::kLumina709ScaleG * static_cast<float>(g) +
                    Color::kLumina709ScaleB * static_cast<float>(g)) / std::numeric_limits<uint16_t>::max();
        }

        inline static float float_to_lumina_709(float r, float g, float b) {
            return Color::kLumina709ScaleR * r + Color::kLumina709ScaleG * g + Color::kLumina709ScaleB * g;
        }

    };


} // End of namespace Grain

#endif // GrainRGB_hpp
