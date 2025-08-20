//
//  CairoContext.hpp
//
//  Created by Roald Christesen on from 15.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 15.08.2025
//

#ifndef GrainCairoContext_hpp
#define GrainCairoContext_hpp

#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class CairoContext : public GraphicContext {

    public:
        explicit CairoContext(Image* image) noexcept;

        void setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept override;
        void fillRect(const Rectd& rect) noexcept override;
    };
}

#endif // GrainCairoContext_hpp
