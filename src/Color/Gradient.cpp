//
//  Gradient.cpp
//
//  Created by Roald Christesen on from 18.01.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/Gradient.hpp"
#include "Color/RGB.hpp"
#include "Color/HSV.hpp"
#include "Color/OKColor.hpp"
#include "Color/RGBLUT1.hpp"
#include "DSP/LUT1.hpp"
#include "DSP/LevelCurve.hpp"
#include "2d/Rect.hpp"
#include "String/CSVString.hpp"
#include "Math/Math.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/AppleCGContext.hpp"


namespace Grain {

    GradientStop::GradientStop() noexcept {
    }


    GradientStop::GradientStop(float pos, const RGBA& color) noexcept {

        m_pos = pos;
        m_colors[0] = color;
    }


    GradientStop::GradientStop(float pos, const RGBA& color1, const RGBA& color2) noexcept {

        m_pos = pos;
        m_colors[0] = color1;
        m_colors[1] = color2;
        m_two_colored = true;
    }


    GradientStop::GradientStop(const GradientStop* stop) noexcept {

        *this = stop;
    }


    GradientStop::~GradientStop() noexcept {
    }


    Gradient::Gradient() noexcept : Object()  {

        _init();
    }


    Gradient::Gradient(const Gradient* gradient) noexcept : Object() {

        _init();
        set(gradient);
    }


    Gradient::~Gradient() noexcept {
        delete m_lut;

        #if defined(__APPLE__) && defined(__MACH__)
            if (m_cg_gradient != nullptr) {
                CGGradientRelease(m_cg_gradient);
            }
        #endif
    }


    Gradient* Gradient::createByPreset(Preset palette, bool flip) noexcept {

        static RGBA _colors[][16] = {
                {
                        // Fast
                        { 14, 14, 120, 255, 255 },
                        { 39, 55, 153, 255, 255 },
                        { 55, 94, 187, 255, 255 },
                        { 70, 133, 215, 255, 255 },
                        { 85, 171, 234, 255, 255 },
                        { 118, 202, 241, 255, 255 },
                        { 159, 226, 237, 255, 255 },
                        { 205, 239, 215, 255, 255 },
                        { 236, 230, 171, 255, 255 },
                        { 244, 207, 125, 255, 255 },
                        { 240, 177, 97, 255, 255 },
                        { 231, 146, 73, 255, 255 },
                        { 215, 114, 54, 255, 255 },
                        { 197, 82, 40, 255, 255 },
                        { 174, 54, 35, 255, 255 },
                        { 150, 20, 30, 255, 255 }
                },
                {
                        // Blackbody
                        { 0, 0, 0, 255, 255 },
                        { 36, 15, 9, 255, 255 },
                        { 62, 22, 17, 255, 255 },
                        { 90, 27, 22, 255, 255 },
                        { 119, 30, 26, 255, 255 },
                        { 150, 33, 30, 255, 255 },
                        { 180, 38, 34, 255, 255 },
                        { 197, 65, 28, 255, 255 },
                        { 214, 88, 19, 255, 255 },
                        { 228, 112, 7, 255, 255 },
                        { 231, 141, 18, 255, 255 },
                        { 233, 169, 29, 255, 255 },
                        { 233, 195, 39, 255, 255 },
                        { 231, 222, 50, 255, 255 },
                        { 246, 240, 144, 255, 255 },
                        { 255, 255, 255, 255, 255 }
                },
                {
                        // Viridis
                        { 68, 1, 84, 255, 255 },
                        { 72, 26, 108, 255, 255 },
                        { 71, 47, 125, 255, 255 },
                        { 65, 68, 135, 255, 255 },
                        { 57, 86, 140, 255, 255 },
                        { 49, 104, 142, 255, 255 },
                        { 42, 120, 142, 255, 255 },
                        { 35, 136, 142, 255, 255 },
                        { 31, 152, 139, 255, 255 },
                        { 34, 168, 132, 255, 255 },
                        { 53, 183, 121, 255, 255 },
                        { 84, 197, 104, 255, 255 },
                        { 122, 209, 81, 255, 255 },
                        { 165, 219, 54, 255, 255 },
                        { 210, 226, 27, 255, 255 },
                        { 253, 231, 37, 255, 255 }
                },
                {
                        // Inferno
                        { 0, 0, 4, 255, 255 },
                        { 12, 8, 38, 255, 255 },
                        { 36, 12, 79, 255, 255 },
                        { 66, 10, 104, 255, 255 },
                        { 93, 18, 110, 255, 255 },
                        { 120, 28, 109, 255, 255 },
                        { 147, 38, 103, 255, 255 },
                        { 174, 48, 92, 255, 255 },
                        { 199, 62, 76, 255, 255 },
                        { 221, 81, 58, 255, 255 },
                        { 237, 105, 37, 255, 255 },
                        { 248, 133, 15, 255, 255 },
                        { 252, 165, 10, 255, 255 },
                        { 250, 198, 45, 255, 255 },
                        { 242, 230, 97, 255, 255 },
                        { 252, 255, 164, 255, 255 }
                },
                {
                        // Magma
                        { 0, 0, 4, 255, 255 },
                        { 11, 9, 36, 255, 255 },
                        { 32, 17, 75, 255, 255 },
                        { 59, 15, 112, 255, 255 },
                        { 87, 21, 126, 255, 255 },
                        { 114, 31, 129, 255, 255 },
                        { 140, 41, 129, 255, 255 },
                        { 168, 50, 125, 255, 255 },
                        { 196, 60, 117, 255, 255 },
                        { 222, 73, 104, 255, 255 },
                        { 241, 96, 93, 255, 255 },
                        { 250, 127, 94, 255, 255 },
                        { 254, 159, 109, 255, 255 },
                        { 254, 191, 132, 255, 255 },
                        { 253, 222, 160, 255, 255 },
                        { 252, 253, 191, 255, 255 }
                },
                {
                        // Plasma
                        { 13, 8, 135, 255, 255 },
                        { 51, 5, 151, 255, 255 },
                        { 80, 2, 162, 255, 255 },
                        { 106, 0, 168, 255, 255 },
                        { 132, 5, 167, 255, 255 },
                        { 156, 23, 158, 255, 255 },
                        { 177, 42, 144, 255, 255 },
                        { 195, 61, 128, 255, 255 },
                        { 211, 81, 113, 255, 255 },
                        { 225, 100, 98, 255, 255 },
                        { 237, 121, 83, 255, 255 },
                        { 246, 143, 68, 255, 255 },
                        { 252, 166, 54, 255, 255 },
                        { 254, 192, 41, 255, 255 },
                        { 249, 220, 36, 255, 255 },
                        { 240, 249, 33, 255, 255 }
                }
        };

        auto gradient = new (std::nothrow) Gradient();
        if (gradient != nullptr) {
            if (palette >= Preset::First && palette <= Preset::Last) {
                auto p_index = static_cast<int32_t>(palette);
                for (int32_t i = 0; i < 16; i++) {
                    float pos = static_cast<float>(i) / 15.0f;
                    if (flip) {
                        pos = 1.0f - pos;
                    }
                    gradient->addStop(pos, _colors[p_index][i]);
                }
            }
            gradient->updateLUT();
        }

        return gradient;
    }


    void Gradient::_init() noexcept {

        m_stops.setSortCompareFunc((SortCompareFunc)_spotSortCompareFunc);
        needsUpdate();
    }


    void Gradient::set(const Gradient* gradient) noexcept {

        if (gradient != nullptr) {
            removeAllStops();

            for (int32_t i = 0; i < gradient->stopCount(); i++) {
                GradientStop stop(gradient->stopPtrAtIndex(i));
                addStop(stop);
            }

            m_color_space = gradient->m_color_space;
            needsUpdate();
        }
    }


    void Gradient::buildKelvinGradient(float k0, float k1, float s, float v, int32_t resolution) noexcept {

        removeAllStops();

        resolution = std::clamp<int32_t>(resolution, 4, 64);
        RGB rgb;
        HSV hsv;

        for (int32_t i = 0; i < resolution; i++) {

            // Smooth the gradient
            rgb.setKelvin(k0 + (k1 - k0) / static_cast<float>(resolution - 1) * static_cast<float>(i));
            hsv.set(rgb);
            hsv.m_data[1] *= s;
            hsv.m_data[2] *= v;
            rgb = hsv;

            addStop(static_cast<float>(i) / static_cast<float>(resolution - 1), rgb);
        }
    }


    void Gradient::buildHueGradient(float s, float v, int32_t resolution) noexcept {

        removeAllStops();

        resolution = std::clamp<int32_t>(resolution, 4, 64);
        RGB rgb;

        for (int32_t i = 0; i < resolution; i++) {

            // Smooth the gradient
            rgb.setHSV(static_cast<float>(i) / static_cast<float>(resolution - 1), s, v);
            addStop(static_cast<float>(i) / static_cast<float>(resolution - 1), rgb);
        }
    }


    void Gradient::buildHSVGradient(const HSV& hsv1, const HSV& hsv2) noexcept {

        removeAllStops();
        addStop(0, RGB(hsv1));
        addStop(1, RGB(hsv2));
    }


    int32_t Gradient::selectedStopCount() const noexcept {

        int32_t n = 0;

        for (auto stop : m_stops) {
            if (stop.isSelected()) {
                n++;
            }
        }

        return n;
    }


    int32_t Gradient::firstSelectedStopIndex() const noexcept {

        int32_t index = 0;
        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                return index;
            }
            index++;
        }

        return -1;
    }


    int32_t Gradient::lastSelectedStopIndex() const noexcept {

        for (int32_t index = static_cast<int32_t>(m_stops.size()) - 1; index >= 0; index--) {
            auto stop = m_stops.elementAtIndex(index);
            if (stop.isSelected()) {
                return index;
            }
        }

        return -1;
    }


    int32_t Gradient::selectedStopIndices(int32_t* out_first_index, int32_t* out_last_index) const noexcept {

        if (stopCount() < 1) {
            return 0;
        }

        int32_t first_index = firstSelectedStopIndex();
        if (first_index < 0) {
            first_index = 0;
        }

        int32_t last_index = lastSelectedStopIndex();
        if (last_index <= first_index) {
            last_index = stopCount() - 1;
        }

        if (out_first_index) {
            *out_first_index = first_index;
        }

        if (out_last_index) {
            *out_last_index = last_index;
        }

        return last_index - first_index + 1;
    }


    void Gradient::selectAllStops() noexcept {

        for (auto& stop : m_stops) {
            stop.select();
        }
    }


    void Gradient::deselectAllStops() noexcept {

        for (auto& stop : m_stops) {
            stop.deselect();
        }
    }


    void Gradient::revertStopSelection() noexcept {

        for (auto& stop : m_stops) {
            stop.toggleSelection();
        }
    }


    void Gradient::selectStopEach(int32_t step) noexcept {

        int32_t first_index = 0;
        int32_t last_index = 0;

        if (selectedStopIndices(&first_index, &last_index) > 0) {
            int32_t index = first_index;
            int32_t counter = 0;

            while (index++ <= last_index) {

                GradientStop* stop = mutStopPtrAtIndex(index);
                if (stop == nullptr) {
                    break;
                }

                stop->m_selected = counter % step;
                counter++;
            }
        }
    }


    bool Gradient::stepSelectedStops(int32_t step_count) noexcept {

        bool changed = false;

        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                if (stop.m_step_count != step_count) {
                    stop.m_step_count = step_count;
                    changed |= true;
                }
            }
        }

        needsUpdate(changed);

        return changed;
    }


    void Gradient::removeAllStops() noexcept {

        m_stops.clear();
        needsUpdate();
    }


    void Gradient::removeStop(int32_t index) noexcept {

        if (m_stops.removeAtIndex(index) == ErrorCode::None) {
            needsUpdate();
        }
    }


    int32_t Gradient::removeSelectedStops() noexcept {

        int32_t n = 0;

        for (int32_t index = static_cast<int32_t>(m_stops.size()) - 1; index >= 0; index--) {
            auto stop = m_stops.elementAtIndex(index);
            if (stop.isSelected()) {
                m_stops.removeAtIndex(index);
                n++;
            }
        }

        needsUpdate(n > 0);
        return n;
    }


    void Gradient::addStop(float pos, const RGB& rgb) noexcept {

        addStop(pos, RGBA(rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], 1.0f));
    }


    void Gradient::addStop(float pos, const RGBA& rgba) noexcept {

        m_stops.push(GradientStop(pos, rgba));
        needsUpdate();
    }


    void Gradient::addStop(float pos, const RGBA& color1, const RGBA& color2) noexcept {

        m_stops.push(GradientStop(pos, color1, color2));
        needsUpdate();
    }


    void Gradient::addStop(GradientStop& stop) noexcept {

        if (stop != nullptr) {
            m_stops.push(stop);
            needsUpdate();
        }
    }


    GradientStop* Gradient::mutStopPtrAtIndex(int32_t index) noexcept {

        return m_stops.mutElementPtrAtIndex(index);
    }


    const GradientStop* Gradient::stopPtrAtIndex(int32_t index) const noexcept {

        return m_stops.elementPtrAtIndex(index);
    }


    void Gradient::colorAtIndex(int32_t index, int32_t part, RGBA& out_color) const noexcept {

        if (auto stop = stopPtrAtIndex(index)) {
            out_color = stop->color(part);
        }
        else {
            out_color = RGBA::kBlack;
        }
    }


    void Gradient::leftColorAtIndex(int32_t index, RGBA& out_color) const noexcept {

        colorAtIndex(index, 0, out_color);
    }


    void Gradient::rightColorAtIndex(int32_t index, RGBA& out_color) const noexcept {

        if (auto stop = stopPtrAtIndex(index)) {
            out_color = stop->rightColor();
        }
        else {
            out_color = RGBA::kBlack;
        }
    }


    bool Gradient::lookupColor(float pos, RGB& out_color) noexcept {

        RGBA rgba;
        bool result = lookupColor(pos, rgba);
        out_color = rgba;
        return result;
    }


    bool Gradient::lookupColor(float pos, RGBA& out_color) noexcept {

        int32_t stop_count = stopCount();

        if (stop_count < 1) {
            // No stops available, return black as color
            out_color = RGBA::kBlack;
            return false;
        }

        sortStops();

        if (auto first_stop = stopPtrAtIndex(0)) {
            if (pos <= first_stop->m_pos) {
                out_color = first_stop->leftColor();
                return true;
            }
        }

        if (auto last_stop = stopPtrAtIndex(lastStopIndex())) {
            if (pos >= last_stop->m_pos) {
                out_color = last_stop->rightColor();
                return true;
            }
        }

        auto l_stop = stopPtrAtIndex(0);
        for (int32_t i = 1; i < stop_count; i++) {

            auto r_stop = stopPtrAtIndex(i);
            if (r_stop == nullptr) {
                return false;
            }

            if (pos >= l_stop->m_pos && pos <= r_stop->m_pos) {

                LevelCurve level_curve;
                level_curve.setByIntArray(l_stop->m_level_curve_values, 10000);

                float d = r_stop->m_pos - l_stop->m_pos;
                float t = std::fabs(d) > std::numeric_limits<float>::epsilon() ? (pos - l_stop->m_pos) / d : std::numeric_limits<float>::max();
                // ct.setBlend(c1, c2, t);
                out_color.mixbox(l_stop->rightColor(), r_stop->leftColor(), t);
                return true;
            }

            l_stop = r_stop;
        }

        return false;
    }


    bool Gradient::lookupColorValues(float pos, float* out_values) noexcept {

        bool result = false;

        if (out_values != nullptr) {
            RGBA rgba;
            result = lookupColor(pos, rgba);
            rgba.values(out_values);
        }

        return result;
    }


    bool Gradient::setColorAtIndex(int32_t index, int32_t part, const RGBA& color) noexcept {

        if (canAccessStop(index)) {
            sortStops();
            GradientStop* stop = mutStopPtrAtIndex(index);
            if (stop == nullptr) {
                return false;
            }
            stop->setColor(part, color);
            needsUpdate();
            return true;
        }

        return false;
    }


    void Gradient::rememberSelectedStops() noexcept {

        for (int32_t i = 0; i < stopCount(); i++) {
            GradientStop* stop = mutStopPtrAtIndex(i);
            if (stop != nullptr) {
                if (stop->isSelected()) {
                    stop->remember();
                }
            }
        }
    }


    bool Gradient::moveSelectedStops(float delta) noexcept {

        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::lowest();

        for (int32_t i = 0; i < stopCount(); i++) {
            GradientStop* stop = mutStopPtrAtIndex(i);

            if (stop == nullptr) {
                // Fatal error
                return false;
            }

            if (stop->isSelected()) {
                float pos = stop->rememberedPos();
                if (pos < min) {
                    min = pos;
                }
                if (pos > max) {
                    max = pos;
                }
            }

            if (delta < 0 && min <= 0) {
                return false;
            }
            if (delta > 0 && max >= 1) {
                return false;
            }

            if (min + delta < 0) {
                delta = -min;
            }
            else if (max + delta > 1) {
                delta = 1.0f - max;
            }
        }

        for (int32_t i = 0; i < stopCount(); i++) {
            GradientStop* stop = mutStopPtrAtIndex(i);

            if (stop == nullptr) {
                // Fatal error
                return false;
            }

            if (stop->isSelected()) {
                stop->m_pos = stop->rememberedPos() + delta;
                needsUpdate();
            }
        }

        return true;
    }


    bool Gradient::setColorOfSelectedStops(const RGB& color) noexcept {

        bool changed = false;

        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                stop.setColor(0, color);
                changed = true;
            }
        }

        needsUpdate(changed);

        return changed;
    }


    bool Gradient::setColorOfSelectedStops(const RGBA& color) noexcept {

        bool changed = false;

        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                stop.setColor(0, color);
                changed = true;
            }
        }

        needsUpdate(changed);

        return changed;
    }


    bool Gradient::setStepCountOfSelectedStops(int32_t step_count) noexcept {

        bool changed = false;

        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                stop.m_step_count = step_count;
                changed = true;
            }
        }

        needsUpdate(changed);
        return changed;
    }


    bool Gradient::setColorModeOfSetectedStops(bool two_colored) noexcept {

        bool changed = false;

        for (auto& stop : m_stops) {
            if (stop.isSelected()) {
                stop.m_two_colored = two_colored;
                changed = true;
            }
        }

        needsUpdate(changed);
        return changed;
    }


    void Gradient::needsUpdate(bool flag) noexcept {
        if (flag) {
            m_must_sort = true;
            m_lut_must_update = true;
#if defined(__APPLE__) && defined(__MACH__)
            m_cg_gradient_must_update = true;
#endif
        }
    }


    void Gradient::update(GraphicContext* gc) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (m_cg_gradient_must_update) {
            if (m_cg_gradient != nullptr) {
                CGGradientRelease(m_cg_gradient);
            }

            m_cg_gradient = macos_cgGradient(gc);
            m_cg_gradient_must_update = false;
        }
#else
        // TODO: Implement linux version
#endif
    }


    bool Gradient::reset() noexcept {

        removeAllStops();
        addStop(0, RGBA::kBlack);
        addStop(0.5f, RGBA(1, 0, 0.4f, 1), RGBA(0.2f, 0, 1, 0.5f));
        addStop(0.8f, RGBA(0.2f, 0.9, 0.3f, 1));
        addStop(1, RGBA::kWhite);

        return true;
    }


    bool Gradient::flip() noexcept {

        if (stopCount() > 1) {
            for (auto& stop : m_stops) {
                stop.m_pos = 1.0f - stop.m_pos;
            }
            needsUpdate();
            return true;
        }
        else {
            return true;
        }
    }


    bool Gradient::distribute() noexcept {

        int32_t first_index = 0;
        int32_t last_index = 0;

        int32_t range = Gradient::selectedStopIndices(&first_index, &last_index);
        if (range > 2) {
            GradientStop* first_stop = mutStopPtrAtIndex(first_index);
            GradientStop* last_stop = mutStopPtrAtIndex(last_index);

            if (first_stop != nullptr && last_stop != nullptr) {
                float pos = first_stop->m_pos;
                float distance = last_stop->m_pos - pos;
                float step = distance / (range - 1);

                pos += step;

                for (int32_t i = first_index + 1; i < last_index; i++) {
                    GradientStop* stop = mutStopPtrAtIndex(i);
                    if (stop != nullptr) {
                        stop->m_pos = pos;
                    }
                    pos += step;
                }

                needsUpdate();
                return true;
            }
        }

        return false;
    }


    bool Gradient::stretch() noexcept {

        if (stopCount() < 2) {
            return false;
        }

        auto first_stop = mutStopPtrAtIndex(0);
        auto last_stop = mutStopPtrAtIndex(lastStopIndex());

        if (first_stop != nullptr && last_stop != nullptr) {
            float length = last_stop->m_pos - first_stop->m_pos;

            if (length <= 0) {
                return false;
            }

            float t = -first_stop->m_pos;  // Translate
            float s = 1.0f / length;

            for (int32_t i = 0; i < stopCount(); i++) {
                GradientStop* stop = mutStopPtrAtIndex(i);
                if (stop != nullptr) {
                    stop->m_pos = (stop->m_pos + t) * s;
                }
            }
        }

        needsUpdate();
        return true;
    }


#if defined(__APPLE__) && defined(__MACH__)
    CGGradientRef Gradient::macos_cgGradient(GraphicContext* gc) noexcept {
        constexpr int32_t kMaxColorStops = 512;

        if (!gc || gc->magic() != Type::fourcc('m', 'a', 'c', ' ')) {
            return nullptr;
        }

        auto mac_cg = static_cast<AppleCGContext*>(gc);

        int32_t cg_color_count = macos_cgColorCount();
        if (cg_color_count > kMaxColorStops) {
            return nullptr;
        }

        LevelCurve level_curve;

        sortStops();

        // 10 extra stops
        CGFloat cg_gradient_locations[kMaxColorStops + 10];
        CGFloat cg_gradient_colors[kMaxColorStops + 10][4];
        int32_t cg_spot_index = 0;

        auto l_stop = stopPtrAtIndex(0);

        for (int32_t stop_index = 1; stop_index < stopCount(); stop_index++) {

            // std::cout << "stop_index: \n";
            // std::cout << "l_stop: " << l_stop << std::endl;

            auto r_stop = stopPtrAtIndex(stop_index);
            // std::cout << "r_stop: " << r_stop << std::endl;

            RGBA color;
            float cg_stop_location;

            float left_pos = l_stop->m_pos;
            float right_pos = r_stop->m_pos;

            if (l_stop->m_step_count > 0) {
                RGBA left_color = l_stop->rightColor();
                RGBA right_color = r_stop->leftColor();

                int32_t step_n = l_stop->m_step_count;
                float pos = left_pos;
                float pos_step = (right_pos - left_pos) / static_cast<float>(step_n);
                float pos_offset = pos_step / 100;

                for (int32_t step_index = 0; step_index < step_n; step_index++) {
                    cg_gradient_locations[cg_spot_index] = pos;

                    float blend = static_cast<float>(step_index) / static_cast<float>(step_n);
                    // color.setBlend(left_color, right_color, blend);
                    color.mixbox(left_color, right_color, blend);

                    cg_gradient_colors[cg_spot_index][0] = color.red();
                    cg_gradient_colors[cg_spot_index][1] = color.green();
                    cg_gradient_colors[cg_spot_index][2] = color.blue();
                    cg_gradient_colors[cg_spot_index][3] = color.alpha();

                    pos += pos_step;
                    cg_spot_index++;

                    cg_stop_location = pos - pos_offset;
                    cg_gradient_locations[cg_spot_index] = cg_stop_location;
                    cg_gradient_colors[cg_spot_index][0] = color.red();
                    cg_gradient_colors[cg_spot_index][1] = color.green();
                    cg_gradient_colors[cg_spot_index][2] = color.blue();
                    cg_gradient_colors[cg_spot_index][3] = color.alpha();

                    cg_spot_index++;
                }
            }
            else {
                cg_stop_location = l_stop->m_pos;

                RGBA c1 = l_stop->rightColor();
                RGBA c2 = r_stop->leftColor();
                // std::cout << "  c1: " << c1 << ", c2: " << c2 << std::endl;
                RGBA ct;
                level_curve.setByIntArray(l_stop->m_level_curve_values, 10000);
                cg_gradient_locations[cg_spot_index] = cg_stop_location;
                cg_gradient_colors[cg_spot_index][0] = l_stop->rightColor().red();
                cg_gradient_colors[cg_spot_index][1] = l_stop->rightColor().green();
                cg_gradient_colors[cg_spot_index][2] = l_stop->rightColor().blue();
                cg_gradient_colors[cg_spot_index][3] = l_stop->rightColor().alpha();
                // std::cout << cg_spot_index << "  cg_stop_location: " << cg_stop_location << ", color: " << l_stop->rightColor() << std::endl;
                cg_spot_index++;

                for (int32_t j = 1; j < m_cg_resolution; j++) {
                    cg_stop_location = static_cast<float>(Math::remapclamped(0, m_cg_resolution, left_pos, right_pos, j));
                    cg_gradient_locations[cg_spot_index] = cg_stop_location;

                    double blend = level_curve.yAtX(static_cast<double>(j) / static_cast<double>(m_cg_resolution - 1));
                    // ct.setBlend(c1, c2, blend);
                    ct.mixbox(c1, c2, static_cast<float>(blend));

                    // std::cout << cg_spot_index << "  cg_stop_location: " << cg_stop_location << ", color: " << ct << ", blend: " << blend << std::endl;

                    cg_gradient_colors[cg_spot_index][0] = ct.red();
                    cg_gradient_colors[cg_spot_index][1] = ct.green();
                    cg_gradient_colors[cg_spot_index][2] = ct.blue();
                    cg_gradient_colors[cg_spot_index][3] = ct.alpha();

                    cg_spot_index++;
                }

                if (r_stop->m_two_colored) {

                    cg_stop_location = right_pos - ((right_pos - cg_stop_location) / 20.0);
                    cg_gradient_locations[cg_spot_index] = cg_stop_location;
                    cg_gradient_colors[cg_spot_index][0] = c2.red();
                    cg_gradient_colors[cg_spot_index][1] = c2.green();
                    cg_gradient_colors[cg_spot_index][2] = c2.blue();
                    cg_gradient_colors[cg_spot_index][3] = c2.alpha();
                    // std::cout << cg_spot_index << "  m_two_colored cg_stop_location: " << cg_stop_location << ", color: " << c2 << std::endl;
                    cg_spot_index++;
                }
            }

            if (stop_index == stopCount() - 1) {  // Last stop.
                cg_stop_location = r_stop->m_pos;
                cg_gradient_locations[cg_spot_index] = cg_stop_location;
                cg_gradient_colors[cg_spot_index][0] = r_stop->rightColor().red();
                cg_gradient_colors[cg_spot_index][1] = r_stop->rightColor().green();
                cg_gradient_colors[cg_spot_index][2] = r_stop->rightColor().blue();
                cg_gradient_colors[cg_spot_index][3] = r_stop->rightColor().alpha();
                // std::cout << cg_spot_index << "  last cg_stop_location: " << cg_stop_location << ", color: " << r_stop->leftColor() << std::endl;
                cg_spot_index++;
            }

            l_stop = r_stop;
        }

        CGColorSpaceRef cg_color_space = nullptr;
        bool release_cg_color_space = false;

        switch (m_color_space) {
            case ColorSpace::LinearRGB:
                cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceLinearSRGB);
                release_cg_color_space = cg_color_space != nullptr;
                break;

            case ColorSpace::sRGB:
                cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                release_cg_color_space = cg_color_space != nullptr;
            default:
                break;
        }

        if (cg_color_space == nullptr) {
            cg_color_space = mac_cg->cgColorSpace();
        }

        /*
        auto image = Image::createRGBA(1000, 1000);
        GraphicContext gc2(image);
        for (int32_t i = 0; i < cg_spot_index; i++) {
            gc2.setFillColor(cg_gradient_colors[i][0], cg_gradient_colors[i][1], cg_gradient_colors[i][2], cg_gradient_colors[i][3]);
            gc2.fillRect(cg_gradient_locations[i] * 1000, i * 3, 20, 3);
            std::cout << i << ": " << cg_gradient_locations[i] << ": ";
            for (int32_t j = 0; j < 4; j++) {
                if (j > 0) {
                    std::cout << ", ";
                }
                std::cout << cg_gradient_colors[i][j];
            }
            std::cout << std::endl;
        }
        image->writePng("/Users/roaldchristesen/Desktop/grrrr.png");
         */

        CGGradientRef cg_gradient = CGGradientCreateWithColorComponents(cg_color_space,
                                                                        (CGFloat*)cg_gradient_colors,
                                                                        cg_gradient_locations,
                                                                        cg_spot_index);

        if (release_cg_color_space) {
            CFRelease(cg_color_space);
        }

        return cg_gradient;
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    int32_t Gradient::macos_cgColorCount() noexcept {

        // TODO: Check, Test!!!

        int32_t n = 0;
        for (auto stop : m_stops) {
            n += stop.m_step_count > 0 ? stop.m_step_count * 2 : m_cg_resolution;
        }

        return n + 1;   // Add 1 for the very last stop
    }
#endif

    void Gradient::sortStops() noexcept {

        if (m_must_sort) {
            m_stops.sort();
            m_must_sort = false;
            m_lut_must_update = true;
        }
    }


    void Gradient::setLUTResolution(int32_t resolution) noexcept {

        if (m_lut != nullptr) {
            if (resolution != m_lut_resolution) {
                m_lut->setResolution(resolution);
                m_lut_resolution = resolution;
                m_lut_must_update = true;
            }
        }
    }


    bool Gradient::updateLUT() noexcept {

        if (m_lut_must_update) {
            sortStops();
            if (m_lut == nullptr) {
                m_lut = new (std::nothrow) RGBLUT1(m_lut_resolution);
                if (m_lut == nullptr) {
                    return false;
                }
            }

            m_lut->updateByGradient(this);
            m_lut_must_update = false;
        }

        return true;
    }


    bool Gradient::lookupFromLUT(float pos, RGB& out_color) noexcept {

        if (!updateLUT()) {
            return false;
        }
        else {
            m_lut->lookup(pos, out_color);
            return true;
        }
    }


    void Gradient::draw(GraphicContext* gc, const Vec2d& start_pos, const Vec2d& end_pos) noexcept {
        gc->drawGradient(this, start_pos, end_pos);
    }


    void Gradient::draw(GraphicContext* gc, const Vec2d& start_pos, const Vec2d& end_pos, bool draw_before, bool draw_after) noexcept {
        gc->drawGradient(this, start_pos, end_pos, draw_before, draw_after);
    }


    void Gradient::drawInRect(GraphicContext* gc, const Rectd& rect, Direction direction) noexcept {
        drawInRect(gc, rect, direction, true, true);
    }


    void Gradient::drawInRect(GraphicContext* gc, const Rectd& rect, Direction direction, bool draw_before, bool draw_after) noexcept {
        Vec2d start_pos;
        Vec2d end_pos;

        switch (direction) {
            case Direction::LeftToRight:
                start_pos.m_x = rect.m_x;
                start_pos.m_y = rect.m_y;
                end_pos.m_x = rect.x2();
                end_pos.m_y = rect.m_y;
                break;
            case Direction::RightToLeft:
                start_pos.m_x = rect.x2();
                start_pos.m_y = rect.m_y;
                end_pos.m_x = rect.m_x;
                end_pos.m_y = rect.m_y;
                break;
            case Direction::TopToBottom:
                start_pos.m_x = rect.m_x;
                start_pos.m_y = rect.m_y;
                end_pos.m_x = rect.m_x;
                end_pos.m_y = rect.y2();
                break;
            case Direction::BottomToTop:
                start_pos.m_x = rect.m_x;
                start_pos.m_y = rect.y2();
                end_pos.m_x = rect.m_x;
                end_pos.m_y = rect.m_y;
                break;
            case Direction::DiagonalRightUp:
                start_pos.m_x = rect.m_x;
                start_pos.m_y = rect.y2();
                end_pos.m_x = rect.x2();
                end_pos.m_y = rect.m_y;
                break;
            case Direction::DiagonalRightDown:
                start_pos.m_x = rect.m_x;
                start_pos.m_y = rect.m_y;
                end_pos.m_x = rect.x2();
                end_pos.m_y = rect.y2();
                break;
            case Direction::DiagonalLeftUp:
                start_pos.m_x = rect.x2();
                start_pos.m_y = rect.y2();
                end_pos.m_x = rect.m_x;
                end_pos.m_y = rect.m_y;
                break;
            case Direction::DiagonalLeftDown:
                start_pos.m_x = rect.x2();
                start_pos.m_y = rect.m_y;
                end_pos.m_x = rect.m_x;
                end_pos.m_y = rect.y2();
                break;
        }

        draw(gc, start_pos, end_pos, draw_before, draw_after);
    }


    void Gradient::drawRadial(GraphicContext* gc, const Vec2d& pos, double radius, bool draw_before, bool draw_after) noexcept {
        gc->drawRadialGradient(this, pos, radius, draw_before, draw_after);
    }


#if defined(__APPLE__) && defined(__MACH__)
    GradientFunction::GradientFunction(StandardFunctionType function_type) noexcept {

        struct TypedFunction {
            StandardFunctionType type;
            GradientMacOSColorFunc func;
        };

        static TypedFunction typed_functions[static_cast<int32_t>(StandardFunctionType::Count)] = {
                { StandardFunctionType::Gradient, _standardFunc_gradient },
                { StandardFunctionType::GradientAlpha, _standardFunc_gradientAlpha },
                { StandardFunctionType::LUT1, _standardFunc_LUT1 },
                { StandardFunctionType::RGBLUT1, _standardFunc_RGBLUT1 },
                { StandardFunctionType::OKLChHueRamp, _standardFunc_oklchHue },
                { StandardFunctionType::Kelvin, _standardFunc_kelvin }
        };

        auto index = static_cast<int32_t>(function_type);
        if (index >= 0 && index < static_cast<int32_t>(StandardFunctionType::Count)) {
            _m_color_func = typed_functions[index].func;
        }
        else {
            _m_color_func = nullptr;
        }
    }
#else
    GradientFunction::GradientFunction(StandardFunctionType function_type) noexcept {
        // TODO: Implement linux
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    GradientFunction::GradientFunction(GradientMacOSColorFunc func) noexcept {

    }
#endif

    GradientFunction::~GradientFunction() noexcept {

    }


#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_draw(GraphicContext& gc, const Vec2d& start_point, double start_radius, const Vec2d& end_point, double end_radius, bool extend_start, bool extend_end) noexcept {
        /* TODO: !!!!!
        // Set up the function callbacks structure
        CGFunctionCallbacks callbacks = {
                0,              // Version
                _m_color_func,  // Evaluate callback
                nullptr         // Release info callback (optional)
        };

        // Create the function
        CGFloat domain[] = { 0.0, 1.0 };  // Input range for the function (typically 0 to 1)
        CGFloat range[] = {               // Output range (RGBA values)
                0.0, 1.0,  // Red range
                0.0, 1.0,  // Green range
                0.0, 1.0,  // Blue range
                0.0, 1.0   // Alpha range
        };

        CGFunctionRef function = CGFunctionCreate(this, 1, domain, 4, range, &callbacks);
        CGShadingRef shading = nullptr;

        if (start_radius < 0.0) {
            shading = CGShadingCreateAxial(gc.macos_cgColorSpace(), start_point.cgPoint(), end_point.cgPoint(), function, extend_start, extend_end);
        }
        else {
            shading = CGShadingCreateRadial(gc.macos_cgColorSpace(), start_point.cgPoint(), start_radius, end_point.cgPoint(), end_radius, function, extend_start, extend_end);
        }

        if (gc.macos_cgContext() != nullptr) {
            CGContextSetShouldAntialias(gc.macos_cgContext(), true);
            CGContextDrawShading(gc.macos_cgContext(), shading);
        }

        // Cleanup
        CGFunctionRelease(function);
        CGShadingRelease(shading);
        */
    }
#else
    void GradientFunction::_draw(GraphicContext& gc, const Vec2d& start_point, double start_radius, const Vec2d& end_point, double end_radius, bool extend_start, bool extend_end) noexcept {
        // TODO: Implement linux version
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_gradient(void* info_ptr, const CGFloat* in, CGFloat* out) {
        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)
        auto info = (GradientFunction*)info_ptr;
        auto gradient = (Gradient*)info->m_info_ptr;

        RGBA color;
        gradient->lookupColor(t, color);
        out[0] = color.red();
        out[1] = color.green();
        out[2] = color.blue();
        out[3] = color.alpha();
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_gradientAlpha(void* info_ptr, const CGFloat* in, CGFloat* out) {

        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)

        auto info = (GradientFunction*)info_ptr;
        auto gradient = (Gradient*)info->m_info_ptr;

        RGBA rgba;
        gradient->lookupColor(t, rgba);
        out[0] = rgba.red();
        out[1] = rgba.green();
        out[2] = rgba.blue();
        out[3] = rgba.alpha();
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_LUT1(void* info_ptr, const CGFloat* in, CGFloat* out) {

        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)

        auto info = (GradientFunction*)info_ptr;
        auto lut = (LUT1*)info->m_info_ptr;

        auto vars = info->_m_vars;
        auto alpha = vars[0];

        float value = 0.0f;
        if (lut != nullptr) {
            value = lut->lookup(t);
        }

        out[0] = value;
        out[1] = value;
        out[2] = value;
        out[3] = alpha;
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_RGBLUT1(void* info_ptr, const CGFloat* in, CGFloat* out) {

        static int aa = 0;

        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)

        auto info = (GradientFunction*)info_ptr;
        auto lut = (RGBLUT1*)info->m_info_ptr;

        auto vars = info->_m_vars;
        auto alpha = vars[0];

        if (aa == 0) {
            std::cout << "_standardFunc_RGBLUT1: " << lut->className() << ", " << lut->resolution() << std::endl;
        }

        if (lut != nullptr) {
            RGB color;
            lut->lookup(t, color);
            if (aa < 100) {
                std::cout << aa << ": " << color << std::endl;
            }
            out[0] = color.red();
            out[1] = color.green();
            out[2] = color.blue();
        }
        out[3] = alpha;

        aa++;
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_oklchHue(void* info_ptr, const CGFloat* in, CGFloat* out) {

        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)

        auto info = (GradientFunction*)info_ptr;

        auto vars = info->_m_vars;
        auto hue_start = vars[0] / 360;
        auto hue_end = vars[1] / 360;
        auto lightness = vars[2];
        auto chroma = vars[3];
        auto alpha = vars[4];
        double hue = Math::remap(0, 1, hue_start, hue_end, t);

        OKLCh lch(lightness, chroma, hue);
        RGB rgb(lch);
        out[0] = rgb.red();
        out[1] = rgb.green();
        out[2] = rgb.blue();
        out[3] = alpha;
    }
#endif

#if defined(__APPLE__) && defined(__MACH__)
    void GradientFunction::_standardFunc_kelvin(void* info_ptr, const CGFloat* in, CGFloat* out) {

        /* TODO !!!!!
        CGFloat t = in[0];  // Gradient position (0.0 to 1.0)

        auto info = (GradientFunction*)info_ptr;

        auto vars = info->_m_vars;
        // auto k1 = vars[GradientFunction::kVar_Kelvin1];
        // auto k2 = vars[GradientFunction::kVar_Kelvin2];

        // TODO: Use k1 and k2 to render the section defined by caller!
        auto lut = App::kelvinLUT();
        RGB rgb;
        lut->lookup(t, rgb);

        out[0] = rgb.red();
        out[1] = rgb.green();
        out[2] = rgb.blue();
        out[3] = 1.0f;
         */
    }
#endif


}  // End of namespace Grain
