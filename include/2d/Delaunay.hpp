//
//  Delaunay.hpp
//
//  Created by Roald Christesen on from 07.06.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  Implementation of https://github.com/delfrrr/delaunator-cpp/blob/master/include/delaunator.hpp


#ifndef GrainDelaunay_hpp
#define GrainDelaunay_hpp

#include"Math/Vec2.hpp"
#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "Math/Vec2.hpp"
#include "2d/RangeRect.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace Grain {

/*
// Kahan and Babuska summation, Neumaier variant; accumulates less FP error
inline double sum(const std::vector<double>& x) {
    double sum = x[0];
    double err = 0.0;

    for (size_t i = 1; i < x.size(); i++) {
        const double k = x[i];
        const double m = sum + k;
        err += std::fabs(sum) >= std::fabs(k) ? sum - m + k : k - m + sum;
        sum = m;
    }
    return sum + err;
}
 */


template<typename T, typename U>
class Delaunay : public Object {
public:
    static constexpr T kInvalidIndex = std::numeric_limits<T>::max();

    struct Compare {
        const List<Vec2<U>>& vertex_list;
        Vec2<U> pos;

        bool operator()(T a, T b) const {
            const Vec2<U>& a_pos = vertex_list.elementAtIndex(a);
            const Vec2<U>& b_pos = vertex_list.elementAtIndex(b);

            double d1 = pos.squaredDistance(a_pos);
            double d2 = pos.squaredDistance(b_pos);

            if (d1 != d2) return d1 < d2;
            if (a_pos.x_ != b_pos.x_) return a_pos.x_ < b_pos.x_;
            return a_pos.y_ < b_pos.y_;
        }
    };

    // Helper for sorting by angle
    struct AngleCenter {
        U angle;
        Vec2<U> center;
    };

public:
    List<T> halfedges_;
    List<T> hull_prev_;
    List<T> hull_next_;
    List<T> hull_tri_;
    T hull_start_;
    bool flip_y_ = true;   // If set to `true`, the y-axis is pointing downwards
    Rect<U> bounds_;
    bool clip_at_bounds_flag_ = false;
    bool invalid_bounds_flag_ = false;

private:
    const List<Vec2<U>>& vertex_list_;
    List<T> triangle_indices_;
    List<T> hash_;
    Vec2<U> center_;
    T hash_size_;
    List<T> edge_stack_;
    T voronoi_cell_count_ = 0;
    Vec2<U>** voronoi_cells_ = nullptr;
    T* voronoi_cell_sizes_ = nullptr;

public:
    Delaunay(const List<Vec2<U>>& vertex_list) : vertex_list_(vertex_list) {
    }

    ~Delaunay() {
        freeVoronoiCells();
    }

    void setBounds(const Rect<U>& bounds);
    void update();
    double hullArea();


    const T triangleVertexCount() const noexcept { return static_cast<T>(vertex_list_.size()); }
    const List<Vec2<U>>& triangleVertexList() const noexcept { return vertex_list_; }

    const List<T>& triangleIndexList() const noexcept { return triangle_indices_; }
    inline const Vec2<U>& triangleVertexAtIndex(T index) const noexcept {
        return vertex_list_[triangle_indices_[index]];
    }

    const T voronoiCellCount() const noexcept { return voronoi_cell_count_; }
    Vec2<U>* const* voronoiCells() const noexcept { return voronoi_cells_; }
    const T* voronoiCellSizes() const noexcept { return voronoi_cell_sizes_; }

    void computeVoronoiCells() noexcept;

    bool inside(const Vec2<U>& v, Edge edge);
    Vec2<U> intersect(const Vec2<U>& a, const Vec2<U>& b, Edge edge);
    T clipAgainstEdge(const Vec2<U>* vertices, T vertex_n, Edge edge, Vec2<U>* out_vertices);
    T clipPolygonToRect(const Vec2<U>* vertices, T vertex_n, Vec2<U>* out_vertices);

private:
    T legalize(T a);
    inline T hashKey(const Vec2<U>& pos) const;
    T addTriangle(T i0, T i1, T i2, T a, T b, T c);
    void link(const T a, const T b);
    inline bool isClockwise(const Vec2<U>& a, const Vec2<U>& b, const Vec2<U>& c);
    static inline int32_t compareAngle(const void* a, const void* b);
    static inline T fastMod(const T i, const T c);
    void freeVoronoiCells();
};


// Standard types
using Delaunay64d = Delaunay<int64_t, double>;
using Delaunay32d = Delaunay<int32_t, double>;
using Delaunay64f = Delaunay<int64_t, float>;
using Delaunay32f = Delaunay<int32_t, float>;


} // End of namespace Grain

#endif //GrainDelaunay_hpp