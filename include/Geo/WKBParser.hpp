//
//  WKBParser.hpp
//
//  Created by Roald Christesen on from 26.02.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

//  TODO: Support for ..
//  Point
//  LineString
//  Polygon
//  MultiPoint
//  MultiLineString
//  MultiPolygon
//  GeometryCollection
//  CircularString
//  CompoundCurve
//  CurvePolygon
//  MultiCurve
//  MultiSurface
//  PointZ (3D Point)
//  PolygonZ (3D Polygon)

#ifndef GrainWKBParser_hpp
#define GrainWKBParser_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Math/Vec2.hpp"


namespace Grain {

    class WKBParser : Object {

    public:
        enum class WKBType {
            Undefined = -1,
            Point = 1,
            LineString = 2,
            Polygon = 3,
            MultiPoint = 4,
            MultiLineString = 5,
            MultiPolygon = 6,

            First = Point,
            Last = MultiPolygon
        };

    protected:
        WKBType m_type = WKBType::Undefined;
        bool m_little_endian = false;
        bool m_binary_mode = false;

        const char* m_text_data = nullptr;
        int32_t m_text_length = 0;
        int32_t m_read_pos = 0;

        const uint8_t* m_binary_data = nullptr;
        const uint8_t* m_binary_ptr;
        int32_t m_binary_size = 0;

    public:
        WKBParser() {
        }

        ~WKBParser() {
        }

        [[nodiscard]] const char* typeName() const noexcept {
            static const char* names[] = { "Point", "LineString", "Polygon", "MultiPoint", "MultiLineString", "MultiPolygon" };
            static const char* undefined_name = "Undefined";
            if (m_type >= WKBType::First && m_type <= WKBType::Last) {
                return names[(int32_t)m_type - (int32_t)WKBType::First];
            }
            else {
                return undefined_name;
            }
        }

        void setBinaryData(const uint8_t* data, int32_t size) noexcept;
        void setTextData(const char* data, int32_t size) noexcept;

        [[nodiscard]] bool isPoint() const noexcept { return m_type == WKBType::Point; }
        [[nodiscard]] bool isLineString() const noexcept { return m_type == WKBType::LineString; }
        [[nodiscard]] bool isPolygon() const noexcept { return m_type == WKBType::Polygon; }
        [[nodiscard]] bool isMultiPoint() const noexcept { return m_type == WKBType::MultiPoint; }
        [[nodiscard]] bool isMultiLineString() const noexcept { return m_type == WKBType::MultiLineString; }
        [[nodiscard]] bool isMultiPolygon () const noexcept { return m_type == WKBType::MultiPolygon ; }

        [[nodiscard]] uint8_t readNibble();
        [[nodiscard]] uint8_t readByte();
        [[nodiscard]] uint32_t readInt();
        [[nodiscard]] double readDouble();
        void readVec2(Vec2d& out_vec);
    };


} // End of namespace Grain

#endif // GrainWKBParser_hpp
