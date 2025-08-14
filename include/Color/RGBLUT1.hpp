//
//  RGBLUT1.hpp
//
//  Created by Roald Christesen on from 05.02.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainRGBLUT1_hpp
#define GrainRGBLUT1_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"


namespace Grain {

    class Gradient;
    class RGB;


    class RGBLUT1 : public Object {
    public:
        enum {
            kMinResolution = 2,
            kMaxResolution = 8192,
            kDefaultResolution = 512
        };

    protected:
        int32_t m_resolution = 0;
        int32_t m_max_resolution = 0;
        int32_t m_max_index = 0;
        RGB* m_samples = nullptr;

        static const RGBLUT1* g_kelvin_lut;

    public:
        RGBLUT1(int32_t resolution) noexcept;
        ~RGBLUT1() noexcept;

        const char* className() const noexcept { return "RGBLUT1"; }

        int32_t resolution() const noexcept { return m_resolution; }
        int32_t maxResolution() const noexcept { return m_max_resolution; }
        int32_t maxIndex() const noexcept { return m_max_index; }
        const RGB* samplesPtr() const noexcept { return m_samples; }
        RGB* mutSamplesPtr() const noexcept { return m_samples; }

        bool setResolution(int32_t resolution) noexcept;
        void setColorAtIndex(int32_t index, const RGB& color) noexcept;

        bool shrink() noexcept;
        void updateByGradient(Gradient* gradient) noexcept;
        void smooth() noexcept;
        void lookup(float pos, RGB& out_color) const noexcept;
        void lookup(float pos, float* out_color) const noexcept;

        static const RGBLUT1 *kelvinLUT() noexcept { return g_kelvin_lut; }
        static const RGBLUT1 *_initKelvinLUT() noexcept;
    };


} // End of namespace Grain

#endif // GrainRGBLUT1_hpp
