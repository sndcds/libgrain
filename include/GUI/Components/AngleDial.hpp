//
//  AngleDial.hpp
//
//  Created by Roald Christesen on 11.12.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainAngleDial_hpp
#define GrainAngleDial_hpp

#include "GUI/Components/ValueComponent.hpp"
#include "2d/Ring.hpp"


namespace Grain {

    class AngleDial : public ValueComponent {
    protected:
        Ring<double> ring_{};

    public:
        explicit AngleDial(const Rectd& rect) noexcept;
        ~AngleDial() noexcept override = default;

        [[nodiscard]] const char* className() const noexcept override { return "AngleDial"; }

        static AngleDial* add(View* view, const Rectd& rect);


        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;

        void handleMouseDown(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;

        void handleScrollWheel(const Event& event) noexcept override;

        void updateRepresentations(const Component* excluded_component) noexcept override;
    };
} // End of namespace

#endif // GrainAngleDial_hpp