//
//  CairoContext.hpp
//
//  Created by Roald Christesen on from 15.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#ifndef GrainCairoContext_hpp
#define GrainCairoContext_hpp

#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class CairoContext : public GraphicContext {

    public:
        // explicit CairoContext(Component* component) = default;
        explicit CairoContext() noexcept;
        // explicit CairoContext(PDFWriter* pdfWriter) = default;

        ~CairoContext() noexcept;

        void log(Log& l) const noexcept;


        void _freeCairoResources() noexcept;

        void setImage(Image* image) noexcept override;

        void setFillColor(float r, float g, float b, float alpha) noexcept override;
        void setFillRGB(const RGB& rgb) noexcept override;
        void setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept override;
      
        void fillRect(double x, double y, double width, double height) noexcept override;
        void fillRect(const Rectd& rect) noexcept override;
       
        void strokeRect(double x, double y, double width, double height) noexcept override;

        [[nodiscard]] void* cairoSurface() { return _m_cairo_surface; }
        [[nodiscard]] void* cairoContext() { return _m_cairo_cr; }

    };
}

#endif // GrainCairoContext_hpp
