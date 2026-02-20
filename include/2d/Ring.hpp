//
// Ring.hpp
//
// Created by Roald Christesen on 08.01.2026
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainRing_hpp
#define GrainRing_hpp

#include "Math/Vec2.hpp"


namespace Grain {

    template <class T>
    class Ring {
    public:
        Vec2<T> center_;
        T inner_radius_;
        T outer_radius_;

    public:
        Ring() noexcept = default;
        virtual ~Ring() noexcept = default;

        [[nodiscard]] virtual const char* className() const noexcept {
            return "Ring";
        }

        friend std::ostream& operator << (std::ostream& os, const Ring* o) {
            o == nullptr ? os << "Line nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Ring& o) {
            return os << "center_: " << o.center_ <<
                ", inner_radius_: " << o.inner_radius_ <<
                ", outer_radius_: " << o.outer_radius_<< std::endl;
        }

        [[nodiscard]] Vec2d center() const noexcept { return center_; }
        [[nodiscard]] T innerRadius() const noexcept { return inner_radius_; }
        [[nodiscard]] T outerRadius() const noexcept { return outer_radius_; }

        [[nodiscard]] double area() const noexcept {
            return std::numbers::pi * (outer_radius_ * outer_radius_ - inner_radius_ * inner_radius_);
        }

        [[nodiscard]] bool contains(const Vec2<T>& pos) const noexcept {
            Vec2d diff = pos - center_;
            double dist2 = diff.squaredLength();
            return dist2 >= inner_radius_ * inner_radius_ &&
                   dist2 <= outer_radius_ * outer_radius_;
        }

        [[nodiscard]] bool contains(const Vec2<T>& pos, T tolerance) const noexcept {
            Vec2d diff = pos - center_;
            double dist2 = diff.squaredLength();
            double inner = inner_radius_ - tolerance;
            double outer = outer_radius_ + tolerance;
            return dist2 >= inner * inner && dist2 <= outer * outer;
        }
    };

    // Standard types
    using Ringi = Ring<int32_t>;    ///< 32 bit integer
    using Ringl = Ring<int64_t>;    ///< 64 bit integer
    using Ringf = Ring<float>;      ///< 32 bit floating point
    using Ringd = Ring<double>;     ///< 64 bit floating point

} // End of namespace

#endif // GrainRing_hpp