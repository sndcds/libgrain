//
//  ColorWell.hpp
//
//  Created by Roald Christesen on 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainColorWell_hpp
#define GrainColorWell_hpp

#include "GUI/Components/Component.hpp"
#include "Color/HSV.hpp"

namespace Grain {

    class ColorWell : public Component {
    protected:
        RGB color_;

    public:
        ColorWell(const Rectd& rect) noexcept;
        virtual ~ColorWell() noexcept;

        const char* className() const noexcept override { return "ColorWell"; }


        static ColorWell* add(View* view, const Rectd& rect);


        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;
        void setByComponent(Component* component) noexcept override;
        void setColor(const RGB& color) noexcept override {
            color_ = color;
            needsDisplay();
        }

        RGB color() const noexcept override { return color_; }
        HSV hsvColor() const noexcept { return HSV(color_); }

        void setHSVColor(const HSV& hsv) noexcept {
            color_.setHSV(hsv.data_[0], hsv.data_[1], hsv.data_[2]);
            needsDisplay();
        }
    };

 } // End of namespace

#endif // GrainColorWell_hpp