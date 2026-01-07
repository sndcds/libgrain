//
//  SVGPaintServer.hpp
//
//  Created by Roald Christesen on 11.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPaintServer_hpp
#define GrainSVGPaintServer_hpp

#include "Type/Object.hpp"
#include "String/String.hpp"


namespace Grain {

    class SVGPaintServer : public Object {
        friend class SVG;

    public:
        enum class PaintServerType {
            Underfined = -1,
            Gradient = 0,
            Pattern,
            Hatches,
            Image
        };

    protected:
        PaintServerType server_type_ = PaintServerType::Underfined;
        String id_;
        String class_;
        String style_;
        String href_;

    public:
        SVGPaintServer() noexcept = default;
        ~SVGPaintServer() noexcept = default;
    };
    
 } // End of namespace

#endif // GrainSVGPaintServer_hpp