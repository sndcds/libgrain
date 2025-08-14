//
//  Superellipse.hpp
//
//  Created by Roald Christesen on from 03.05.25.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainSuperellipse_hpp
#define GrainSuperellipse_hpp

#include "Grain.hpp"
#include "Bezier/Bezier.hpp"


namespace Grain {


    class Superellipse {
    public:
        Superellipse() noexcept {}
        ~Superellipse() noexcept {}

        virtual const char* className() const noexcept { return "Superellipse"; }

        friend std::ostream& operator << (std::ostream& os, const Superellipse* o) {
            o == nullptr ? os << "Superellipse nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Superellipse& o) {
            return os;
        }

        static Vec2d posAtT(double t, double a, double b, double n) noexcept;
        static Vec2d tangentAtT(double t, double a, double b, double n) noexcept;

        // Helper functions
        static inline double _deg2rad(double degrees) { return degrees * std::numbers::pi / 180.0; }
        static inline double _sign(double v) { return (v > 0) - (v < 0); }
    };


} // End of namespace Grain

#endif // GrainSuperellipse_hpp
