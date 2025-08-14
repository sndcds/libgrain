//
//  ComponentStyle.hpp
//
//  Created by Roald Christesen on from 28.07.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"
#include "GUI/Style.hpp"
#include "Graphic/Font.hpp"


namespace Grain {

    StyleClass::StyleClass() {
        // Set all property types
        for (int32_t i = 0; i < kPropertyCount; i++) {
            m_properties[i].m_type = static_cast<StylePropertyType>(i);
        }
    }

    void StyleClass::build(const StyleClass* parent_style_class, StyleList* style_list) noexcept {

        if (style_list != nullptr) {
            // Set all style properties from `style_list` and the corresponding flags
            for (auto& p : *style_list->properties()) {
                auto index = static_cast<int>(p.type());
                m_properties[index].setValue(p.value());
                m_state_flags.setFlag(index);
            }
        }

        // Get all properties which are not set from `parent_style_class`
        for (int32_t i = 0; i < kPropertyCount; i++) {
            //if (!m_state_flags.isSet(index)) {
                m_properties[i] = parent_style_class->m_properties[i];
            //}
        }
    }


    void StyleSet::build(const StyleSet* parent_style_set) noexcept {
        std::cout << "StyleSet::build m_view_style_class\n";
        m_view_style_class.build(&parent_style_set->m_view_style_class, nullptr);
        std::cout << "StyleSet::build m_button_style_class\n";
        m_button_style_class.build(&parent_style_set->m_button_style_class, nullptr);
    }


} // End of namespace Grain
