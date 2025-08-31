//
//  GraphicCompoundPath.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 23.08.2025
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
        ObjectList<GraphicPath*> m_paths;
        Vec2d m_offset;
        bool m_must_add_path = true;


    public:
        GraphicCompoundPath() noexcept;
        ~GraphicCompoundPath() noexcept;

        [[nodiscard]] virtual const char* className() const noexcept { return "GraphicCompoundPath"; }

        friend std::ostream& operator << (std::ostream& os, const GraphicCompoundPath* o) {
            o == nullptr ? os << "GraphicCompoundPath nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GraphicCompoundPath& o) {
            os << "GraphicCompoundPath:";
            os << "\npath count: " << o.pathCount();
            os << "\noffset: " << o.m_offset;
            return os;
        }

        virtual void log(Log& l) const;


        [[nodiscard]] bool hasPaths() const noexcept { return m_paths.size() > 0; }
        [[nodiscard]] int32_t pathCount() const noexcept { return (int32_t)m_paths.size(); };
        [[nodiscard]] int32_t lastPointIndex() const noexcept { return (int32_t)m_paths.size() - 1; }
        [[nodiscard]] GraphicPath* pathPtrAtIndex(int32_t index) noexcept;
        [[nodiscard]] GraphicPath* lastPathPtr() noexcept;
        bool boundsRect(Rectd& out_bounds_rect) const noexcept;
        double polygonCentroid(Vec2d& out_centroid) const noexcept;

        void clear() noexcept;
        void clearOffset() noexcept { m_offset.zero(); }

        ErrorCode addEmptyPath(int32_t point_capacity = 32) noexcept;

        Rectd buildFromText(const Font& font, const char* text) noexcept;
        Rectd buildFromText(const Font& font, const String& text) noexcept;

        Rectd buildFromWKB(WKBParser& wkb_parser, RemapRectd& remap_rect) noexcept;

        /* TODO: macOS
        void _addCGPathElement(const CGPathElement* element) noexcept;
        */

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
