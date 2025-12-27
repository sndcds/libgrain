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

#include "GUI/Components/Component.hpp"


namespace Grain {

class View : public Component {
    friend class Component;

public:
    explicit View(int32_t tag = 0) noexcept;
    explicit View(const Rectd& rect, int32_t tag = 0) noexcept;
    virtual ~View() noexcept;

    [[nodiscard]] const char* className() const noexcept override { return "View"; }


    void _init(const Rectd &rect) noexcept;

    virtual View* addView() noexcept { return addView(Rectd(0, 0, 100, 100)); }
    virtual View* addView(const Rectd& rect) noexcept;
    Component* addComponent(Component* component, AddFlags flags = AddFlags::kNone) noexcept;
    void removeComponent(Component* component) noexcept;

    [[nodiscard]] ObjectList<Component*> components() noexcept { return components_; }
    [[nodiscard]] bool hasDescendant(const Component* component) noexcept override;

    void deselectRadioGroup(int32_t radio_group) noexcept override;

    void geometryChanged() noexcept override; // TODO: Implement!!!!!
    void setSplitViewFlag(bool flag) noexcept { split_view_flag_ = flag; }

    void draw(const Rectd& dirty_rect) noexcept override;

protected:
    Component* first_key_component_ = nullptr;
    Component* curr_key_component_ = nullptr;

    ObjectList<Component*> components_;
    bool split_view_flag_ = false;
    Rectd drag_rect_;

    GUIStyle style_set_;
};


} // End of namespace Grain

#endif // GrainView_hpp
