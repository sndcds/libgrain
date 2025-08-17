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
#include "String/String.hpp"


namespace Grain {


    template <class T>
    class Dimension {

    public:
        T m_width = 0;
        T m_height = 0;

    public:
        Dimension() noexcept {}
        explicit Dimension(T width, T height) noexcept : m_width(width), m_height(height) {}
        explicit Dimension(const char* csv) noexcept { setByCSV(csv); }
        explicit Dimension(const String& csv) noexcept;


        virtual const char* className() const noexcept { return "Dimension"; }

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

        T width() const noexcept { return m_width; }
        T height() const noexcept { return m_height; }
        T centerX() const noexcept { return m_width / 2; }
        T centerY() const noexcept { return m_height / 2; }
        Vec2<T> center() const noexcept { return Vec2<T>(m_width / 2, m_height / 2); }

        int64_t roundedWidth() noexcept { return static_cast<int64_t>(std::round(m_width)); }
        int64_t roundedHeight() noexcept { return static_cast<int64_t>(std::round(m_height)); }
        T area() const noexcept { return m_width * m_height; }
        double aspectRatio() const noexcept {
            if (std::fabs(m_width) > std::numeric_limits<double>::epsilon() &&
                m_height > std::numeric_limits<double>::epsilon()) {
                return static_cast<double>(m_height) / static_cast<double>(m_width);
            }
            return DBL_MAX;
        }
        bool isLandscape() const noexcept { return aspectRatio() < 1.0; }
        bool isPortrait() const noexcept { return aspectRatio() > 1.0; }
        bool isSquare() const noexcept { return std::fabs(1.0 - aspectRatio() <= std::numeric_limits<float>::epsilon()); }



        void scale(T scale) noexcept {
            m_width *= scale;
            m_height *= scale;
        }

        /* TODO: macOS specific! Implement in another file!
         void setCGSizeInt32(CGSize size) noexcept { m_width = (int32_t)std::round(size.width); m_height = (int32_t)std::round(size.height); }
         */

        void set(T value) noexcept { m_width = value; m_height = value; }
        void set(T width, T height) noexcept { m_width = width; m_height = height; }
        void zero() noexcept { m_width = m_height = 0; }

        bool setByCSV(const char* csv) noexcept  {
            #warning "Dimension.setByCSV() must be implemented"
            return false;
        }

        bool setByCSV(const String& csv) noexcept  {
            return setByCSV(csv.utf8());
        }
    };


    // Standard types
    using Dimensioni = Dimension<int32_t>;  ///< 32 bit integer
    using Dimensionl = Dimension<int64_t>;  ///< 64 bit integer
    using Dimensionf = Dimension<float>;    ///< 32 bit floating point
    using Dimensiond = Dimension<double>;   ///< 64 bit floating point

    // Specialized methods
    template <> bool Dimensioni::setByCSV(const char* csv) noexcept;
    template <> bool Dimensionl::setByCSV(const char* csv) noexcept;
    template <> bool Dimensionf::setByCSV(const char* csv) noexcept;
    template <> bool Dimensiond::setByCSV(const char* csv) noexcept;


} // End of namespace Grain

#endif // GrainDimension_hpp
