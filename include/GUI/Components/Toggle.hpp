//
//  Toggle.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainToggle_hpp
#define GrainToggle_hpp

#include "GUI/Components/Checkbox.hpp"


namespace Grain {

    class Toggle : public Checkbox {

    public:
        Toggle(const Rectd& rect, int32_t tag = 0) noexcept;
        ~Toggle() noexcept override;

        [[nodiscard]] const char* className() const noexcept override{ return "Toggle"; }

        static Toggle* add(View* view, const Rectd& rect, int32_t tag = 0);

        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;
    };


} // End of namespace Grain

#endif // GrainToggle_hpp
