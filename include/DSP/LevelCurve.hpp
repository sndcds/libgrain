//
//  LevelCurve.hpp
//
//  Created by Roald Christesen on 08.04.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainLevelCurve_hpp
#define GrainLevelCurve_hpp

#include "Type/Object.hpp"
#include "Math/Vec2.hpp"


namespace Grain {

    class LevelCurve {
    public:
        double m_y1, m_x2, m_y2, m_x3, m_y3, m_y4;
        double m_amount;

    public:
        LevelCurve();
        LevelCurve(float y1, float x2, float y2, float x3, float y3, float y4);
        ~LevelCurve();

        virtual const char* className() const noexcept { return "LevelCurve"; }

        friend std::ostream& operator << (std::ostream &os, const LevelCurve* o) {
            o == nullptr ? os << "LevelCurve nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const LevelCurve &o) {
            os << o.m_y1 << ", " << o.m_x2 << ", " << o.m_y2 << ", " << o.m_x3 << ", " << o.m_y3 << ", " << o.m_y4;
            return os;
        }

        void reset() noexcept;
        void setCurve(const LevelCurve* curve) noexcept;
        void set(float y1, float x2, float y2, float x3, float y3, float y4) noexcept;
        void setByIntArray(const int32_t* array, int32_t max_level) noexcept;

        Vec2d pointAtIndex(int32_t index) const noexcept;
        bool resetPointAtIndex(int32_t index) noexcept;

        double _bezierComponent(double t) const noexcept;
        double _bezierComponentDerivative(double t) const noexcept;
        double yAtX(double x, int32_t max_iter = 20, double epsilon = 1e-6) const noexcept;
    };


} // End of namespace Grain

#endif // GrainLevelCurve_hpp
