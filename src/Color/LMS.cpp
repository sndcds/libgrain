//
//  LMS.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//
//  GrLMS.cpp
//  GrainLib
//
//  Created by Roald Christesen on 25.01.24.
//

#include "Color/LMS.hpp"
#include "Color/RGB.hpp"
#include "Type/Type.hpp"


namespace Grain {

    const Mat3f LMS::g_from_ciexyz_matrices[kMethodCount] = {
            Mat3f(0.40024f, 0.7076f, -0.08081f, -0.2263f, 1.16532f, 0.0457f, 0.0f,     0.0f,    0.91822f),  // VonKries
            Mat3f(0.8951f,  0.2664f, -0.1614f,  -0.7502f, 1.7135f,  0.0367f, 0.0389f, -0.0685f, 1.0296f),   // Bradford
            Mat3f(1.2694f, -0.0988f, -0.1706f,  -0.8364f, 1.8006f,  0.0357f, 0.0297f, -0.0315f, 1.0018f),   // Sharp
            Mat3f(0.7982f,  0.3389f, -0.1371f,  -0.5918f, 1.5512f,  0.0406f, 0.0008f,  0.239f,  0.9753f),   // CMCCAT2000
            Mat3f(0.7328f,  0.4296f, -0.1624f,  -0.7036f, 1.6975f,  0.0061f, 0.0030f,  0.0136f, 0.9834f)    // CAT02
    };

    const Mat3f LMS::g_to_ciexyz_matrices[kMethodCount] = {
            Mat3f(1.859936f, -1.129382f, 0.219897f, 0.361191f, 0.638812f, -0.000006f, -0.0f,       0.0f,      1.089064f),   // VonKries
            Mat3f(0.986993f, -0.147054f, 0.159963f, 0.432305f, 0.518360f,  0.049291f, -0.008529f,  0.040043f, 0.968487f),   // Bradford
            Mat3f(0.815633f,  0.047155f, 0.137217f, 0.379114f, 0.576942f,  0.044001f, -0.012260f,  0.016743f, 0.995519f),   // Sharp
            Mat3f(1.062305f, -0.256743f, 0.160018f, 0.407920f, 0.550236f,  0.034437f, -0.100833f, -0.134626f, 1.016755f),   // CMCCAT2000
            Mat3f(1.096124f, -0.278869f, 0.182745f, 0.454369f, 0.473533f,  0.072098f, -0.009628f, -0.005698f, 1.015326f)    // CAT02
    };


    LMS::LMS(const RGB& rgb) {
        m_data[0] = 0.4122214708 * rgb.m_data[0] + 0.5363325363 * rgb.m_data[1] + 0.0514459929 * rgb.m_data[2];
        m_data[1] = 0.2119034982 * rgb.m_data[0] + 0.6806995451 * rgb.m_data[1] + 0.1073969566 * rgb.m_data[2];
        m_data[2] = 0.0883024619 * rgb.m_data[0] + 0.2817188376 * rgb.m_data[1] + 0.6299787005 * rgb.m_data[2];
    }


    const Mat3f LMS::fromCIEXYZMatrix(LMS::Method method) {
        return g_from_ciexyz_matrices[std::clamp<int32_t>(static_cast<int32_t>(method), 0, kMethodCount)];
    }


    const Mat3f LMS::toCIEXYZMatrix(LMS::Method method) {
        return g_to_ciexyz_matrices[std::clamp<int32_t>(static_cast<int32_t>(method), 0, kMethodCount)];
    }


}  // End of namespace Grain
