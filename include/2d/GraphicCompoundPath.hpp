//
//  GraphicCompoundPath.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainGraphicCompoundPath_hpp
#define GrainGraphicCompoundPath_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "2d/Rect.hpp"
#include "2d/GraphicPath.hpp"


namespace Grain {

class StrokeStyle;
class GraphicContext;
class Font;
class String;
class WKBParser;


class GraphicCompoundPath : protected Object {

    friend class GraphicPath;
    friend class GraphicPathPoint;
    friend class GraphicContext;

protected:
    ObjectList<GraphicPath*> paths_;
    Vec2d offs_;
    bool must_add_path_ = true;

public:
    GraphicCompoundPath() noexcept;
    ~GraphicCompoundPath() noexcept override;

    [[nodiscard]] const char* className() const noexcept override {
        return "GraphicCompoundPath";
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicCompoundPath* o) {
        o == nullptr ? os << "GraphicCompoundPath nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicCompoundPath& o) {
        os << "path count: " << o.pathCount();
        os << ", offset: " << o.offs_;
        return os;
    }

    [[nodiscard]] bool hasPaths() const noexcept { return paths_.size() > 0; }
    [[nodiscard]] int32_t pathCount() const noexcept { return static_cast<int32_t>(paths_.size()); };
    [[nodiscard]] int32_t lastPointIndex() const noexcept { return static_cast<int32_t>(paths_.size()) - 1; }
    [[nodiscard]] GraphicPath* pathPtrAtIndex(int32_t index) noexcept;
    [[nodiscard]] GraphicPath* lastPathPtr() noexcept;
    bool boundsRect(Rectd& out_bounds_rect) const noexcept;
    double polygonCentroid(Vec2d& out_centroid) const noexcept;

    void clear() noexcept;
    void clearOffset() noexcept { offs_.zero(); }

    ErrorCode addEmptyPath(int32_t point_capacity = 32) noexcept;

    Rectd buildFromText(const Font& font, const char* text) noexcept;
    Rectd buildFromText(const Font& font, const String& text) noexcept;

    Rectd buildFromWKB(WKBParser& wkb_parser, RemapRectd& remap_rect) noexcept;


#if defined(__APPLE__) && defined(__MACH__)
    void _addCGPathElement(const CGPathElement* element) noexcept;
#endif


    void finish() noexcept;

    void projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix = nullptr) noexcept;

    void addAllPaths(GraphicContext* gc) noexcept;
    void fill(GraphicContext* gc) noexcept;
    void fillOuter(GraphicContext* gc) noexcept;
    void fillEvenOdd(GraphicContext* gc) noexcept;
    void stroke(GraphicContext* gc, StrokeStyle* stroke_style = nullptr) noexcept;
    void addClip(GraphicContext* gc) noexcept;
};


} // End of namespace Grain

#endif // GrainGraphicCompoundPath_hpp
