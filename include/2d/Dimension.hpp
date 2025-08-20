//
//  Dimension.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainDimension_hpp
#define GrainDimension_hpp

#include "Grain.hpp"
#include "2d/Rect.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    template <class T>
    class Dimension {

    public:
        T m_width = 0;
        T m_height = 0;

    public:
        Dimension() noexcept : m_width(static_cast<T>(0)), m_height(static_cast<T>(0)) {}
        Dimension(T width, T height) noexcept : m_width(width), m_height(height) {}
        explicit Dimension(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
        explicit Dimension(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }


        [[nodiscard]] virtual const char* className() const noexcept { return "Dimension"; }

        friend std::ostream& operator << (std::ostream& os, const Dimension* o) {
            o == nullptr ? os << "Dimension nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Dimension& o) {
            os << o.m_width << ", " << o.m_height;
            return os;
        }

        Dimension& operator = (const Dimension<int32_t>& r) {
            m_width = (T)r.m_width; m_height = (T)r.m_height; return *this;
        }

        Dimension& operator = (const Dimension<int64_t>& r) {
            m_width = (T)r.m_width; m_height = (T)r.m_height; return *this;
        }

        Dimension& operator = (const Dimension<float>& r) {
            m_width = (T)r.m_width; m_height = (T)r.m_height; return *this;
        }

        Dimension& operator = (const Dimension<double>& r) {
            m_width = (T)r.m_width; m_height = (T)r.m_height; return *this;
        }


        Dimension& operator = (const Rect<T>& v) { m_width = v.m_width; m_height = v.m_height; return *this; }

        /* TODO: macOS specific! Implement in another file!
         Dimension& operator = (NSRect v) { m_width = v.size.width; m_height = v.size.height; return *this; }
         */

        bool operator == (const Dimension& v) const { return m_width == v.m_width && m_height == v.m_height; }
        bool operator != (const Dimension& v) const { return m_width != v.m_width || m_height != v.m_height; }

        [[nodiscard]] T width() const noexcept { return m_width; }
        [[nodiscard]] T height() const noexcept { return m_height; }
        [[nodiscard]] T centerX() const noexcept { return m_width / 2; }
        [[nodiscard]] T centerY() const noexcept { return m_height / 2; }
        [[nodiscard]] Vec2<T> center() const noexcept { return Vec2<T>(m_width / 2, m_height / 2); }

        [[nodiscard]] int64_t roundedWidth() noexcept { return static_cast<int64_t>(std::round(m_width)); }
        [[nodiscard]] int64_t roundedHeight() noexcept { return static_cast<int64_t>(std::round(m_height)); }
        [[nodiscard]] T area() const noexcept { return m_width * m_height; }
        [[nodiscard]] double aspectRatio() const noexcept {
            if (std::fabs(m_width) > std::numeric_limits<double>::epsilon() &&
                m_height > std::numeric_limits<double>::epsilon()) {
                return static_cast<double>(m_height) / static_cast<double>(m_width);
            }
            return std::numeric_limits<double>::max();
        }
        [[nodiscard]] bool isLandscape() const noexcept { return aspectRatio() < 1.0; }
        [[nodiscard]] bool isPortrait() const noexcept { return aspectRatio() > 1.0; }
        [[nodiscard]] bool isSquare() const noexcept { return std::fabs(1.0 - aspectRatio() <= std::numeric_limits<float>::epsilon()); }

        void set(T value) noexcept { m_width = value; m_height = value; }
        void set(T width, T height) noexcept { m_width = width; m_height = height; }
        void zero() noexcept { m_width = m_height = 0; }

        bool setByCSV(const char* csv, char delimiter = ',') noexcept {
            if (csv) {
                CSVLineParser csv_line_parser(csv);
                csv_line_parser.setDelimiter(delimiter);
                T values[2]{};
                if (csv_line_parser.values(2, values) == 2) {
                    m_width = values[0];
                    m_height = values[1];
                    return true;
                }
            }
            return false;
        }

        bool setByCSV(const String& csv) noexcept  {
            return setByCSV(csv.utf8());
        }

        void flip() noexcept {
            std::swap(m_width, m_height);
        }

        void scale(T scale) noexcept {
            m_width *= scale;
            m_height *= scale;
        }
    };


    // Standard types
    using Dimensioni = Dimension<int32_t>;  ///< 32 bit integer
    using Dimensionl = Dimension<int64_t>;  ///< 64 bit integer
    using Dimensionf = Dimension<float>;    ///< 32 bit floating point
    using Dimensiond = Dimension<double>;   ///< 64 bit floating point

} // End of namespace Grain

#endif // GrainDimension_hpp
