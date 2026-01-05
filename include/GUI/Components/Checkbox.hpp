//
//  Checkbox.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 17.08.2025
//

#ifndef GrainCheckbox_hpp
#define GrainCheckbox_hpp

#include "GUI/Components/Button.hpp"


namespace Grain {

    class Checkbox : public Button {

    protected:
        float check_size_ = 12.0;

    public:
        Checkbox(const Rectd& rect, const char* text, int32_t tag = 0) noexcept;
        ~Checkbox() noexcept override = default;

        [[nodiscard]] const char* className() const noexcept override { return "Checkbox"; }


        static Checkbox* add(View* view, const Rectd& rect, int32_t tag = 0);
        static Checkbox* add(View* view, const Rectd& rect, const char* text, int32_t tag = 0);
        static Checkbox* add(View* view, const Rectd& rect, const char* text, int32_t radio_group, int32_t radio_value, int32_t tag = 0);

        [[nodiscard]] int32_t selectedRadioValue() const noexcept;

        void draw(GraphicContext* gc, const Rectd &dirty_rect) noexcept override;
    };


} // End of namespace Grain

#endif // GrainCheckbox_hpp
