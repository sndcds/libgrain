//
//  OKColor.hpp
//
//  Created by Roald Christesen on from 19.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainOKColor_hpp
#define GrainOKColor_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    class RGB;
    class OKLCh;


    /**
     *  @brief OKLab Color.
     *
     *  OKLab represents colors using:
     *  - L = Lightness (0 (black) to ?? (diffuse white).
     *  - a = green (at negative -- is there a lower bound?) to red (positive).
     *  - b = blue (at negative) to yellow (at positive).
     */
    class OKLab {

    public:
        float m_data[3] = { 0.0f, 0.0f, 0.0f };

    public:
        OKLab() noexcept {}
        OKLab(float l, float a, float b) noexcept : m_data { l, a, b } {}
        OKLab(const OKLCh& oklch) noexcept;
        OKLab(const RGB& rgb) noexcept;

        ~OKLab() noexcept {}


        virtual const char* className() const noexcept { return "OKLab"; }

        friend std::ostream& operator << (std::ostream& os, const OKLab* o) {
            o == nullptr ? os << "OKLab nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const OKLab& o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
            return os;
        }


        bool operator == (const OKLab& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const OKLab& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }

        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        float lumina() const noexcept { return m_data[0]; }
        float a() const noexcept { return m_data[1]; }
        float b() const noexcept { return m_data[2]; }

        void Lightness(float l) noexcept { m_data[0] = l; }
        void setA(float c) noexcept { m_data[1] = c; }
        void setB(float h) noexcept { m_data[2] = h; }

        void set(float l, float a, float b) noexcept { m_data[0] = l; m_data[1] = a; m_data[2] = b; }

        OKLab blend(const OKLab& oklab, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            OKLab result;
            result.m_data[0] = m_data[0] * ti + oklab.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + oklab.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + oklab.m_data[2] * t;
            return result;
        }
    };


    /**
     *  @brief OKLCh Color.
     *
     *  OKLCh represents colors using:
     *  - L (Lightness): Perceptual lightness
     *  - C (Chroma): Chroma representing chromatic intensity, with values from 0
     *                (achromatic) with no upper limit, but in practice not exceeding +0.5; CSS treats +0.4 as 100%.
     *  - h (Hue): Hue angle in a color wheel, typically denoted in decimal degrees.
     */
    class OKLCh {

    public:
        float m_data[3] = { 0.0f, 0.0f, 0.0f };

    public:
        OKLCh() noexcept {}
        OKLCh(float l, float c, float h) noexcept : m_data { l, c, h } {}
        OKLCh(const OKLab& oklab) noexcept;
        OKLCh(const RGB& rgb) noexcept;

        ~OKLCh() noexcept {}


        virtual const char* className() const noexcept { return "OKLCh"; }

        friend std::ostream& operator << (std::ostream& os, const OKLCh* o) {
            o == nullptr ? os << "OKLCh nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const OKLCh& o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
            return os;
        }


        bool operator == (const OKLCh& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const OKLCh& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }

        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        float lightness() const noexcept { return m_data[0]; }
        float chroma() const noexcept { return m_data[1]; }
        float hue() const noexcept { return m_data[2]; }

        void setLightness(float l) noexcept { m_data[0] = l; }
        void setChroma(float c) noexcept { m_data[1] = c; }
        void setHue(float h) noexcept { m_data[2] = h; }

        void set(float l, float c, float h) noexcept { m_data[0] = l; m_data[1] = c; m_data[2] = h; }

        OKLCh blend(const OKLCh& oklch, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            OKLCh result;
            result.m_data[0] = m_data[0] * ti + oklch.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + oklch.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + oklch.m_data[2] * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainOKColor_hpp
