//
//  CIEXYZ.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainCIEXYZ_hpp
#define GrainCIEXYZ_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"
#include "Math/Vec2.hpp"


namespace Grain {

    class RGB;
    class CIExyY;

    /**
     *  @brief CIE XYZFile Color.
     *
     *  The CIE XYZFile color space represents colors using three components:
     *  - X: The tristimulus value for redness or greenness.
     *  - Y: The tristimulus value for lightness or brightness.
     *  - Z: The tristimulus value for yellowness or blueness.
     *
     *  CIE XYZFile provides a device-independent color model that serves as a foundation for other color spaces.
     *  It aids in color matching, color transformation, and color analysis across various applications.
     */
    class CIEXYZ {

    public:
        enum class RGBMatrixType {
            D50_sRGB,
            D50_CIE,
            D50_Adobe1998,
            D50_Apple,
            D50_KodakProPhoto,
            D65_sRGB,
            D65_Adobe1998,
            D65_Apple
        };

        enum {
            kRGBMatrixCount = 8
        };


    protected:
        static constexpr float g_min_temperature = 1666.7f;

    public:
        float m_data[3]{};

    public:
        CIEXYZ() noexcept {}
        CIEXYZ(float x, float y, float z) noexcept : m_data { x, y, z } {}
        CIEXYZ(const RGB& rgb) noexcept;
        CIEXYZ(const CIExyY& xyY) noexcept;

        ~CIEXYZ() noexcept {}


        virtual const char* className() const noexcept { return "CIEXYZ"; }

        friend std::ostream& operator << (std::ostream& os, const CIEXYZ* o) {
            o == nullptr ? os << "CIEXYZ nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const CIEXYZ& o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
            return os;
        }


        bool operator == (const CIEXYZ& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const CIEXYZ& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }


        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        float xValue() const noexcept { return m_data[0]; }
        float yValue() const noexcept { return m_data[1]; }
        float zValue() const noexcept { return m_data[2]; }


        void setX(float x) noexcept { m_data[0] = x; }
        void setY(float y) noexcept { m_data[1] = y; }
        void setZ(float z) noexcept { m_data[2] = z; }

        void set(float x, float y, float z) noexcept { m_data[0] = x; m_data[1] = y; m_data[2] = z; }
        void setKelvin(float temperature) noexcept;

        Vec2f CIExy() const noexcept;
        bool colorTemp(float* temperature) const noexcept;

        void transform(const Mat3f& matrix, RGB& out_rgb) const noexcept;

        CIEXYZ blend(const CIEXYZ& xyz, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            CIEXYZ result;
            result.m_data[0] = m_data[0] * ti + xyz.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + xyz.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + xyz.m_data[2] * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainCIEXYZ_hpp
