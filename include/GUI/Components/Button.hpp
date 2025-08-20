//
//  Button.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 27.07.2025
//

#ifndef GrainButton_hpp
#define GrainButton_hpp

#include "Grain.hpp"
#include "Component.hpp"
#include "GUI/Views/View.hpp"


namespace Grain {

    class Button : public Component {

    public:
        Button(const Rectd& rect, const char* text) noexcept : Button(rect, text, 0) {}
        Button(const Rectd& rect, const char* text, int32_t tag) noexcept;
        virtual ~Button() noexcept {}

        [[nodiscard]] const char* className() const noexcept override { return "Button"; }


        static Button* add(View* view, const Rectd& rect, const char* text, int32_t tag = 0);

        void setSelected(bool selected) noexcept override;


        [[nodiscard]] int32_t radioGroup() const noexcept override { return m_radio_group; }
        [[nodiscard]] int32_t radioValue() const noexcept override { return m_radio_value; }

        void setRadioGroup(int32_t radio_group) noexcept override { m_radio_group = radio_group; }
        void setRadioValue(int32_t radio_value) noexcept override { m_radio_value = radio_value; }

        void draw(const Rectd& dirty_rect) noexcept override;

        void handleMouseDown(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;

        [[nodiscard]] bool isPopUpButton() const noexcept { return m_type == ComponentType::PopUpButton; }

    protected:
        int32_t m_radio_group = 0;
        int32_t m_radio_value = 0;
    };


} // End of namespace Grain

#endif // GrainButton_hpp
