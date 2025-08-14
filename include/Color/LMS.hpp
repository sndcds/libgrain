//
//  LMS.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainLMS_hpp
#define GrainLMS_hpp

#include "Math/Mat3.hpp"


namespace Grain {

    class RGB;


    /**
     *  @brief LMS Color System Description.
     *
     *  The LMS color system is based on three components:
     *  - L: Long-wavelength cone response, sensitive to red-green differences.
     *  - M: Medium-wavelength cone response, sensitive to green-yellow differences.
     *  - S: Short-wavelength cone response, sensitive to blue-violet differences.
     *
     *  LMS is used to model human color vision and perception, offering insights into how the brain processes colors.
     *  It helps explain color blindness and informs color space transformations in various applications.
     */
    class LMS {

    public:
        enum class Method {
            VonKries = 0,
            Bradford,
            Sharp,
            CMCCAT2000,
            CAT02
        };

        enum {
            kMethodCount = 5
        };


    public:
        static const Mat3f g_from_ciexyz_matrices[kMethodCount];
        static const Mat3f g_to_ciexyz_matrices[kMethodCount];

    public:
        float m_data[3]{};

    public:
        LMS() noexcept {}
        LMS(float l, float m, float s) noexcept : m_data { l, m, s } {}
        LMS(const RGB& rgb);

        bool operator == (const LMS& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const LMS& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }


        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        float lValue() const noexcept { return m_data[0]; }
        float mValue() const noexcept { return m_data[1]; }
        float sValue() const noexcept { return m_data[2]; }

        static const Mat3f fromCIEXYZMatrix(LMS::Method method);
        static const Mat3f toCIEXYZMatrix(LMS::Method method);

        void setL(float l) noexcept { m_data[0] = l; }
        void setM(float m) noexcept { m_data[1] = m; }
        void setS(float s) noexcept { m_data[2] = s; }
        void set(float l, float m, float s) noexcept { m_data[0] = l; m_data[1] = m; m_data[2] = s; }

        LMS blend(const LMS& lms, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            LMS result;
            result.m_data[0] = m_data[0] * ti + lms.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + lms.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + lms.m_data[2] * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainLMS_hpp
