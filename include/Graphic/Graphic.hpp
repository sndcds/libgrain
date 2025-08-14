//
//  Graphic.hpp
//
//  Created by Roald Christesen on from 25.07.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainGraphic_hpp
#define GrainGraphic_hpp

namespace Grain {

    enum class DrawMode {
        Undefined = -1,
        Fill = 0,
        Stroke = 1,
        FillStroke = 2,
        StrokeFill = 3
    };

    enum class StrokeJoinStyle {
        Miter = 0,
        Round,
        Bevel
    };

    enum class StrokeCapStyle {
        Butt = 0,
        Round,
        Square
    };

    enum class FillWindingRule {
        NoneZero = 0,
        EvenOdd
    };


    class Graphic {
    };

} // End of namespace Grain

#endif //GrainGraphic_hpp