//
//  Geometry.hpp
//
//  Created by Roald Christesen on 14.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainGeometry_hpp
#define GrainGeometry_hpp


#include <stdint.h>
#include <iostream>
#include <cmath>


namespace Grain {

    /**
     *  @brief Length unit.
     */
    enum class LengthUnit : int32_t {
        Undefined = -1,
        Pixel,              ///< px
        Millimeter,         ///< mm
        QuarterMillimeter,  ///< q, used in CSS
        Centimeter,         ///< cm
        Decimeter,          ///< dm
        Meter,              ///< m
        Decameter,          ///< dam
        Hectometer,         ///< hm
        Kilometer,          ///< km
        Megameter,          ///< Mm
        Gigameter,          ///< Gm
        Micrometer,         ///< μm,
        Nanometer,          ///< nm
        Picometer,          ///< pm
        Inch,               ///< in
        Foot,               ///< ft
        Yard,               ///< yd
        Mile,               ///< mi
        League,             ///< lea
        Fathom,             ///< fath
        Chain,              ///< ch
        Rod,                ///< rd
        Link,               ///< li
        Furlong,            ///< fur
        AstronomicalUnit,   ///< AU
        LightYear,          ///< ly
        Parsec,             ///< pc
        Cubit,              ///< cbt
        NauticalMile,       ///< nmi
        Hand,               ///< hd
        Finger,             ///< fing
        Span,               ///< span
        Palm,               ///< palm
        Ell,                ///< ell
        Fingerbreadth,      ///< fb
        Pace,               ///< pace
        Point,              ///< pt
        Pica,               ///< pica
        Barleycorn,         ///< bc
        Angstrom,           ///< Å
        GeoDegrees          ///< Unit representing angular measurement in degrees, typically used for geographic coordinates.
    };

    struct LengthUnitInfo {
        LengthUnit m_unit;
        const char* m_name;
        const char* m_abbreviation;
        double m_scale_to_meter;        // If < 0, no absolute length calculation is possible.
    };

    /**
     *  @brief Alignment constants.
     */
    enum class Alignment {
        No = 0,
        Center,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        TopLeft,
        Zero
    };

    /**
     *  @brief Direction constants.
     */
    enum class Direction {
        LeftToRight = 0,
        RightToLeft,
        TopToBottom,
        BottomToTop,
        DiagonalRightUp,
        DiagonalRightDown,
        DiagonalLeftUp,
        DiagonalLeftDown
    };

    enum class Edge {
        Top = 0, Right, Bottom, Left
    };

    enum class Corner {
        TopLeft = 0, TopRight, BottomRight, BottomLeft
    };

    enum {
        kEdgeFlagTop = 0x1,
        kEdgeFlagRight = 0x2,
        kEdgeFlagBottom = 0x4,
        kEdgeFlagLeft = 0x8,
        kEdgeFlagAll = 0xF
    };

    enum {
        kCornerFlagTopLeft = 0x1,
        kCornerFlagTopRight = 0x2,
        kCornerFlagBottomRight = 0x4,
        kCornerFlagBottomLeft = 0x8,
        kCornerFlagAll = 0xF
    };

    /**
     *  @brief Enumeration describing the fitting mode.
     */
    enum class FitMode {
        Cover = 0,
        //!< Scale the image proportionally to completely fill the frame, cropping any parts of the image that exceed the frame's boundaries. This ensures that the entire frame is filled with the image, but some parts of the image may be cropped.
        Fit,
        //!< Scale the image proportionally to fit entirely within the frame, without cropping. This ensures that the entire image is visible within the frame, but there may be empty space around the image if the aspect ratios do not match.
        Stretch,
        //!< Scale the image non-proportionally to completely fill the frame without maintaining the aspect ratio. This may distort the image if the frame's aspect ratio differs significantly from the image's aspect ratio.
        Center,
        //!< Position the image at the center of the frame without scaling. This does not resize the image but simply centers it within the frame, potentially leaving empty space around the image.
    };


    class Geometry {
    public:
        [[nodiscard]] static bool isLengthUnit(LengthUnit unit);
        [[nodiscard]] static const char* lengthUnitName(LengthUnit unit);
        [[nodiscard]] static double convertLength(double value, LengthUnit src_unit, LengthUnit dst_unit);
        [[nodiscard]] static double mm_to_inch(double mm) { return mm / 25.4; }
        [[nodiscard]] static double mm_to_pt(double mm) { return mm / 25.4 * 72.0; }
        [[nodiscard]] static double inch_to_mm(double inch) { return inch * 25.4; }
        [[nodiscard]] static double inch_to_pt(double inch) { return inch * 72.0; }
        [[nodiscard]] static double pt_to_inch(double pt) { return pt /  72.0; }
        [[nodiscard]] static double pt_to_mm(double pt) { return pt / 72.0 * 25.4; }

        [[nodiscard]] static inline bool isEdge(Edge edge) {
            return static_cast<int32_t>(edge) >= 0 && static_cast<int32_t>(edge) < 4;
        }
        [[nodiscard]] static inline Edge sanitizedEdge(Edge edge) {
            return static_cast<int32_t>(edge) >= 0 && static_cast<int32_t>(edge) < 4 ? edge : Edge::Top;
        }

        [[nodiscard]] static double shortestAngleOnCircle(double a, double b) noexcept;
        [[nodiscard]] static double normalizeAngle(double angle) noexcept;

    private:
        static const LengthUnitInfo g_length_unit_infos[];
        static const LengthUnit _g_last_unit = LengthUnit::Angstrom;
    };


} // End of namespace Grain

#endif // GrainGeometry_hpp
