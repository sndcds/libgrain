//
//  View.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 26.07.2025
//

#ifndef GrainView_hpp
#define GrainView_hpp

#include "GUI/Component.hpp"


namespace Grain {

    class View : public Component {
    public:
        explicit View(int32_t tag = 0) noexcept;
        explicit View(const Rectd& rect, int32_t tag = 0) noexcept;
        virtual ~View() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "View"; }


        void _init(const Rectd &rect) noexcept;

        virtual View* addView(const Rectd& rect) noexcept;
        Component* addComponent(Component* component, AddFlags flags) noexcept;

        [[nodiscard]] ObjectList<Component*> components() noexcept { return m_components; }
        [[nodiscard]] bool hasDescendant(const Component* component) noexcept override;

        void deselectRadioGroup(int32_t radio_group) noexcept override;

        void draw(const Rectd& dirty_rect) noexcept override;


    protected:
        Component* m_first_key_component = nullptr;
        Component* m_curr_key_component = nullptr;

        ObjectList<Component*> m_components;
        bool m_split_view_flag = false;
        Rectd m_drag_rect;

        // Style
        GUIStyle m_style_set;
    };


} // End of namespace Grain

#endif // GrainView_hpp
