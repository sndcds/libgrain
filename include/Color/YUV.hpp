//
//  YUV.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainYUV_hpp
#define GrainYUV_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    class RGB;


    /**
     *  @brief YUV Color.
     *
     *  The YUV color model represents colors using three components:
     *  - Y (Luma): The brightness or intensity of the color.
     *  - U: The chrominance along the blue-yellow axis.
     *  - V: The chrominance along the red-green axis.
     *
     *  YUV is commonly used in video and image compression, where separating luma (brightness)
     *  from chroma (color) can lead to efficient data representation.
     */
    class YUV {

    public:
        static const Mat3f g_to_rgb_matrix_601;
        static const Mat3f g_to_rgb_matrix_709;
        static const Mat3f g_from_rgb_matrix_601;
        static const Mat3f g_from_rgb_matrix_709;

    public:
        float m_data[3] = { 0.0f, 0.0f, 0.0f };

    public:
        YUV() noexcept {}
        YUV(float y, float u, float v) noexcept : m_data { y, u, v } {}
        YUV(const RGB& rgb, Color::Space color_space = Color::Space::Rec709) noexcept;

        ~YUV() noexcept {}


        virtual const char* className() const noexcept { return "YUV"; }

        friend std::ostream& operator << (std::ostream& os, const YUV* o) {
            o == nullptr ? os << "YUV nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const YUV& o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
            return os;
        }

        bool operator == (const YUV& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const YUV& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }


        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        float yValue() const noexcept { return m_data[0]; }
        float uValue() const noexcept { return m_data[1]; }
        float vValue() const noexcept { return m_data[2]; }


        void setY(float y) noexcept { m_data[0] = y; }
        void setU(float u) noexcept { m_data[1] = u; }
        void setV(float v) noexcept { m_data[2] = v; }
        void set(float y, float u, float v) noexcept { m_data[0] = y; m_data[1] = u; m_data[2] = v; }
        void setRGB601(const RGB& rgb) noexcept;
        void setRGB709(const RGB& rgb) noexcept;

        YUV blend(const YUV& yuv, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            YUV result;
            result.m_data[0] = m_data[0] * ti + yuv.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + yuv.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + yuv.m_data[2] * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainYUV_hpp
