//
//  Geometry.cpp
//
//  Created by Roald Christesen on 14.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include <Geometry.hpp>


namespace Grain {

    /**
     *  @note The abbreviation `pc` can refer to both "pica" (a unit of length in typography)
     *        and "parsec" (an astronomical unit of distance).
     *        Ensure the correct interpretation based on the context of use.
     */
    const LengthUnitInfo Geometry::g_length_unit_infos[] = {
            { LengthUnit::Pixel, "pixel", "px", 0.001 },
            { LengthUnit::Millimeter, "millimeter", "mm", 0.001 },
            { LengthUnit::QuarterMillimeter, "quarter millimeter", "q", 0.00025 },
            { LengthUnit::Centimeter, "centimeter", "cm", 0.01 },
            { LengthUnit::Decimeter, "decimeter", "dm", 0.1 },
            { LengthUnit::Meter, "meter", "m", 1.0 },
            { LengthUnit::Decameter, "decameter", "dam", 10.0 },
            { LengthUnit::Hectometer, "hectometer", "hm", 100.0 },
            { LengthUnit::Kilometer, "kilometer", "km", 1000.0 },
            { LengthUnit::Megameter, "megameter", "Mm", 1000000.0 },
            { LengthUnit::Gigameter, "gigameter", "Gm", 1000000000.0 },
            { LengthUnit::Micrometer, "micrometer", "μm", 0.000001 },
            { LengthUnit::Nanometer, "nanometer", "nm", 0.000000001 },
            { LengthUnit::Picometer, "picometer", "pm", 0.000000000001 },
            { LengthUnit::Inch, "inch", "in", 0.0254 },
            { LengthUnit::Foot, "foot", "ft", 0.3048 },
            { LengthUnit::Yard, "yard", "yd", 0.9144},
            { LengthUnit::Mile, "mile", "mi", 1609.34 },
            { LengthUnit::League, "leauge", "lea", 4828.032 },
            { LengthUnit::Fathom, "fathom", "fath", 1.8288 },
            { LengthUnit::Chain, "chain", "ch", 20.1168 },
            { LengthUnit::Rod, "rod", "rd", 5.0292 },
            { LengthUnit::Link, "link", "li", 0.201168 },
            { LengthUnit::Furlong, "furlong", "fur", 201.168 },
            { LengthUnit::AstronomicalUnit, "astronomical unit", "AU", 149597870700 },
            { LengthUnit::LightYear, "lightyear", "ly", 9.4605e15 },
            { LengthUnit::Parsec, "parsec", "pc", 30856775814671900.0 },
            { LengthUnit::Cubit, "cubit", "cbt", 0.4572 },
            { LengthUnit::NauticalMile, "nautical mile", "nmi", 1852.0 },
            { LengthUnit::Hand, "hand", "hd", 0.1016 },
            { LengthUnit::Finger, "finger", "fing", 0.022225 },
            { LengthUnit::Span, "span", "span", 0.2286 },
            { LengthUnit::Palm, "palm", "palm", 0.0762 },
            { LengthUnit::Ell, "ell", "ell", 1.143 },
            { LengthUnit::Fingerbreadth, "fingerbreadth", "fb", 0.01905 },
            { LengthUnit::Pace, "pace", "pace", 0.762 },
            { LengthUnit::Point, "point", "pt",  0.0254 / 72.0 },
            { LengthUnit::Pica, "pica", "pc",  0.0254 / 6.0 },
            { LengthUnit::Barleycorn, "barleycorn", "bc", 0.0084666667 },
            { LengthUnit::Angstrom, "Angstrom", "Å", 0.0000000001 },
            { LengthUnit::GeoDegrees, "GeoDegrees", "°", -1.0 },
    };


    inline bool Geometry::isLengthUnit(LengthUnit unit) {

        return unit > LengthUnit::Undefined && unit <= _g_last_unit;
    }


    const char* Geometry::lengthUnitName(LengthUnit unit) {
        static const char* undefined_name = "undefined";
        if (isLengthUnit(unit)) {
            return g_length_unit_infos[static_cast<int32_t>(unit)].m_name;
        }
        else {
            return undefined_name;
        }
    }


    double Geometry::convertLength(double value, LengthUnit src_unit, LengthUnit dst_unit) {

        if (src_unit == dst_unit) {
            return value;
        }
        else if (isLengthUnit(src_unit) && isLengthUnit(dst_unit)) {
            return value * g_length_unit_infos[static_cast<int32_t>(src_unit)].m_scale_to_meter / g_length_unit_infos[static_cast<int32_t>(dst_unit)].m_scale_to_meter;
        }
        else {
            return 0.0;
        }
    }


    double Geometry::shortestAngleOnCircle(double a, double b) noexcept {

        double signed_delta = 0.0;
        double raw_delta = a > b ? a - b : b - a;
        double mod_delta = std::fmod(raw_delta, 360.0);
        if (mod_delta > 180.0) {
            signed_delta = 360.0 - mod_delta;    // There is a shorter path in opposite direction
            if (b > a) {
                return -signed_delta;
            }
        }
        else {
            signed_delta = mod_delta;
            if (a > b) {
                return -signed_delta;
            }
        }

        return signed_delta;
    }


    /**
     *  @brief Normalize an angle to be within the range [0, 360).
     *
     *  @param[in,out] angle The input angle to be normalized.
     */
    double Geometry::normalizeAngle(double angle) noexcept {

        angle = std::fmod(angle, 360.0);
        if (angle < 0.0) {
            angle += 360.0;
        }

        return angle;
    }


} // End of namespace Grain
