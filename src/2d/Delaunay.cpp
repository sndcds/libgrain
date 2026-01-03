//
//  Delaunay.cpp
//
//  Created by Roald Christesen on from 07.06.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  Implementation of https://github.com/delfrrr/delaunator-cpp/blob/master/include/delaunator.hpp


#include "2d/Delaunay.hpp"

namespace Grain {

    template<typename T, typename U>
    void Delaunay<T, U>::setBounds(const Rect<U>& bounds) {
        bounds_ = bounds;
        invalid_bounds_flag_ = !bounds.usable();
        clip_at_bounds_flag_ = true;
    }


    template<typename T, typename U>
    void Delaunay<T, U>::update() {
        T n = static_cast<T>(vertex_list_.size());

        halfedges_.clear();
        hull_prev_.clear();
        hull_next_.clear();
        hull_tri_.clear();
        triangle_indices_.clear();
        hash_.clear();;
        edge_stack_.clear();;

        RangeRect<U> range;
        range.initForMinMaxSearch();

        List<T> ids;
        ids.reserve(n);

        {
            int32_t vertex_index = 0;
            for (auto& vertex : vertex_list_) {
                range.add(vertex);
                ids.push(vertex_index);
                vertex_index++;
            }
        }

        Vec2<U> center = range.center();

        double min_dist = std::numeric_limits<double>::max();

        T i0 = kInvalidIndex;
        T i1 = kInvalidIndex;
        T i2 = kInvalidIndex;

        {
            // Pick a vertex close to the centroid
            int32_t vertex_index = 0;
            for (auto& vertex : vertex_list_) {
                const double d = center.distance(vertex);
                if (d < min_dist) {
                    i0 = vertex_index;
                    min_dist = d;
                }
                vertex_index++;
            }
        }

        Vec2<U> i0_pos = vertex_list_.elementAtIndex(i0);

        min_dist = std::numeric_limits<double>::max();

        {
            // Find the point closest to the vertex
            int32_t vertex_index = 0;
            for (auto& vertex : vertex_list_) {
                if (vertex_index != i0) {
                    const double d = i0_pos.distance(vertex);
                    if (d < min_dist && d > 0.0) {
                        i1 = vertex_index;
                        min_dist = d;
                    }
                }
                vertex_index++;
            }
        }

        Vec2<U> i1_pos = vertex_list_.elementAtIndex(i1);

        double min_radius = std::numeric_limits<double>::max();

        {
            // Find the third point which forms the smallest circumcircle with the first two
            int32_t vertex_index = 0;
            for (auto& vertex : vertex_list_) {
                if (vertex_index != i0 && vertex_index != i1) {
                    const double r = Vec2<U>::squaredCircumradius(i0_pos, i1_pos, vertex);
                    if (r < min_radius) {
                        i2 = vertex_index;
                        min_radius = r;
                    }
                }
                vertex_index++;
            }
        }

        if (!(min_radius < std::numeric_limits<double>::max())) {
            throw std::runtime_error("not triangulation");  // TODO: !!!
        }

        Vec2<U> i2_pos = vertex_list_.elementAtIndex(i2);

        if (isClockwise(i0_pos, i1_pos, i2_pos) == true) {
            std::swap(i1, i2);
            std::swap(i1_pos, i2_pos);
        }

        Vec2<U> m_center;
        m_center.circumcenter(i0_pos, i1_pos, i2_pos);

        // Sort the points by distance from the vertex triangle circumcenter
        std::sort(ids.begin(), ids.end(), Compare{ vertex_list_, m_center });

        // Initialize a hash table for storing edges of the advancing convex hull
        hash_size_ = static_cast<T>(std::llround(std::ceil(std::sqrt(n))));
        hash_.resize(hash_size_, kInvalidIndex);

        // Initialize arrays for tracking the edges of the advancing convex hull
        hull_prev_.resize(n, 0);
        hull_next_.resize(n, 0);
        hull_tri_.resize(n, 0);

        hull_start_ = i0;

        T hull_size = 3;

        hull_next_[i0] = hull_prev_[i2] = i1;
        hull_next_[i1] = hull_prev_[i0] = i2;
        hull_next_[i2] = hull_prev_[i1] = i0;

        hull_tri_[i0] = 0;
        hull_tri_[i1] = 1;
        hull_tri_[i2] = 2;

        hash_[hashKey(i0_pos)] = i0;
        hash_[hashKey(i1_pos)] = i1;
        hash_[hashKey(i2_pos)] = i2;

        T max_triangles = n < 3 ? 1 : 2 * n - 5;
        triangle_indices_.reserve(max_triangles * 3);
        halfedges_.reserve(max_triangles * 3);
        addTriangle(i0, i1, i2, kInvalidIndex, kInvalidIndex, kInvalidIndex);

        double epsilon = std::numeric_limits<double>::epsilon();
        Vec2<U> p(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());

        for (T k = 0; k < n; k++) {
            const T i = ids[k];
            const Vec2<U> pos = vertex_list_[i];

            // Skip near-duplicate points
            if (k > 0 && pos.checkEqual(p, epsilon)) {
                continue;
            }
            p = pos;

            // Skip vertex triangle points
            if (p.checkEqual(i0_pos, epsilon) ||
                p.checkEqual(i1_pos, epsilon) ||
                p.checkEqual(i2_pos, epsilon)) {
                continue;
            }

            // Find a visible edge on the convex hull using edge hash
            T start = 0;
            T key = hashKey(p);
            for (T j = 0; j < hash_size_; j++) {
                start = hash_[fastMod(key + j, hash_size_)];
                if (start != kInvalidIndex && start != hull_next_[start]) break;
            }

            start = hull_prev_[start];
            T e = start;
            T q;

            while (static_cast<void>(q = hull_next_[e]), isClockwise(p, vertex_list_[e], vertex_list_[q]) == false) {
                e = q;
                if (e == start) {
                    e = kInvalidIndex;
                    break;
                }
            }

            if (e == kInvalidIndex) {
                continue;  // Likely a near-duplicate point; skip it
            }

            // Add the first triangle from the point
            T t = addTriangle(e, i, hull_next_[e], kInvalidIndex, kInvalidIndex, hull_tri_[e]);

            hull_tri_[i] = legalize(t + 2);
            hull_tri_[e] = t;
            hull_size++;

            // Walk forward through the hull, adding more triangles and flipping recursively
            T next = hull_next_[e];
            while (static_cast<void>(q = hull_next_[next]), isClockwise(p, vertex_list_[next], vertex_list_[q]) == true) {
                t = addTriangle(next, i, q, hull_tri_[i], kInvalidIndex, hull_tri_[next]);
                hull_tri_[i] = legalize(t + 2);
                hull_next_[next] = next;  // mark as removed
                hull_size--;
                next = q;
            }

            // Walk backward from the other side, adding more triangles and flipping
            if (e == start) {
                while (static_cast<void>(q = hull_prev_[e]), isClockwise(p, vertex_list_[q], vertex_list_[e]) == true) {
                    t = addTriangle(q, i, e, kInvalidIndex, hull_tri_[e], hull_tri_[q]);
                    legalize(t + 2);
                    hull_tri_[q] = t;
                    hull_next_[e] = e; // mark as removed
                    hull_size--;
                    e = q;
                }
            }

            // Update the hull indices
            hull_prev_[i] = e;
            hull_start_ = e;
            hull_prev_[next] = i;
            hull_next_[e] = i;
            hull_next_[i] = next;

            hash_[hashKey(p)] = i;
            hash_[hashKey(vertex_list_[e])] = e;
        }
    }

    template<typename T, typename U>
    double Delaunay<T, U>::hullArea() {
        U area = 0.0;
        T e = hull_start_;
        do {
            const Vec2<U>& e_pos = vertex_list_[e];
            const Vec2<U>& prev_pos = vertex_list_[hull_prev_[e]];
            area += (e_pos.x_ - prev_pos.x_) * (e_pos.y_ + prev_pos.y_);
            e = hull_next_[e];
        } while (e != hull_start_);
        return area * 0.5;
    }


    /**
     *  Computes Voronoi cells using malloc-based dynamic memory.
     */
    template<typename T, typename U>
    void Delaunay<T, U>::computeVoronoiCells() noexcept {
        auto point_n = vertex_list_.size();
        auto triangle_n = triangle_indices_.size();

        freeVoronoiCells();

        // Allocate memory for output arrays
        voronoi_cells_ = (Vec2<U>**)malloc(point_n * sizeof(Vec2<U>*));
        voronoi_cell_sizes_ = (T*)calloc(point_n, sizeof(T));

        // Temporary per-site angle-center arrays
        auto temp_storage = (AngleCenter**)malloc(point_n * sizeof(AngleCenter*));
        auto temp_counts = (int32_t*)calloc(point_n, sizeof(int));
        auto temp_capacities = (int32_t*)malloc(point_n * sizeof(int));

        // Initialize arrays
        for (auto i = 0; i < point_n; ++i) {
            temp_capacities[i] = 8;
            temp_storage[i] = (AngleCenter*)malloc(8 * sizeof(AngleCenter));
        }

        // Process triangles
        for (auto i = 0; i < triangle_n; i += 3) {
            auto i0 = triangle_indices_[i];
            auto i1 = triangle_indices_[i + 1];
            auto i2 = triangle_indices_[i + 2];

            Vec2<U> cc;
            cc.circumcenter(vertex_list_[i0], vertex_list_[i1], vertex_list_[i2]);

            for (auto idx : {i0, i1, i2}) {
                auto count = temp_counts[idx];
                if (count >= temp_capacities[idx]) {
                    temp_capacities[idx] *= 2;
                    temp_storage[idx] = (AngleCenter*)realloc(temp_storage[idx], temp_capacities[idx] * sizeof(AngleCenter));
                }
                temp_storage[idx][count] = {static_cast<U>(vertex_list_[idx].angleTo(cc)), cc};
                temp_counts[idx]++;
            }
        }

        // Sort and store results
        Vec2<U> polygon_vertices[256];
        Vec2<U> clipped_polygon_vertices[256];

        voronoi_cell_count_ = 0;
        T cell_index = 0;
        for (auto i = 0; i < point_n; i++) {
            T count = temp_counts[i];
            qsort(temp_storage[i], count, sizeof(AngleCenter), compareAngle);

            for (auto j = 0; j < count; j++) {
                polygon_vertices[j] = temp_storage[i][j].center;
            }
            Vec2<U>* vertex_src = polygon_vertices;

            if (clip_at_bounds_flag_ == true) {
                // Clip polygon to bounds
                count = clipPolygonToRect(polygon_vertices, count, clipped_polygon_vertices);
                vertex_src = clipped_polygon_vertices;
            }

            if (count >= 3) {
                if (count > 256) {
                    std::cout << "count: " << count << std::endl;
                    // TODO; !!!
                    exit(0);
                }
                voronoi_cell_sizes_[cell_index] = count;
                voronoi_cells_[cell_index] = (Vec2<U>*)malloc(count * sizeof(Vec2<U>));
                for (auto j = 0; j < count; j++) {
                    voronoi_cells_[cell_index][j] = vertex_src[j];
                }
                voronoi_cell_count_++;
                cell_index++;
            }

            free(temp_storage[i]);
        }

        // Cleanup
        free(temp_storage);
        free(temp_counts);
        free(temp_capacities);
    }

    template<typename T, typename U>
    bool Delaunay<T, U>::inside(const Vec2<U>& v, Edge edge) {
        switch (edge) {
            case Edge::Left:   return v.x_ >= bounds_.x_;
            case Edge::Right:  return v.x_ <= bounds_.x_ + bounds_.width_;
            case Edge::Top:    return v.y_ >= bounds_.y_;
            case Edge::Bottom: return v.y_ <= bounds_.y_ + bounds_.height_;
            default:           return false;
        }
    }

    template<typename T, typename U>
    Vec2<U> Delaunay<T, U>::intersect(const Vec2<U>& a, const Vec2<U>& b, Edge edge) {
        double dx = b.x_ - a.x_;
        double dy = b.y_ - a.y_;

        switch (edge) {
            case Edge::Left: {
                if (dx == 0.0) return a; // Avoid divide-by-zero
                double x = bounds_.x_;
                double t = (x - a.x_) / dx;
                return Vec2<U>(x, a.y_ + t * dy);
            }
            case Edge::Right: {
                if (dx == 0.0) return a;
                double x = bounds_.x_ + bounds_.width_;
                double t = (x - a.x_) / dx;
                return Vec2<U>(x, a.y_ + t * dy);
            }
            case Edge::Top: {
                if (dy == 0.0) return a;
                double y = bounds_.y_;
                double t = (y - a.y_) / dy;
                return Vec2<U>(a.x_ + t * dx, y);
            }
            case Edge::Bottom: {
                if (dy == 0.0) return a;
                double y = bounds_.y_ + bounds_.height_;
                double t = (y - a.y_) / dy;
                return Vec2<U>(a.x_ + t * dx, y);
            }
        }
        // Fallback: Should never hit if all edges are handled
        return Vec2<U>();
    }

    template<typename T, typename U>
    T Delaunay<T, U>::clipAgainstEdge(const Vec2<U>* vertices, T vertex_n, Edge edge, Vec2<U>* out_vertices) {
        T out_count = 0;
        Vec2<U> prev = vertices[vertex_n - 1];
        bool prev_inside = inside(prev, edge);

        for (auto i = 0; i < vertex_n; i++) {
            Vec2<U> curr = vertices[i];
            bool curr_inside = inside(curr, edge);

            if (curr_inside) {
                if (!prev_inside) {
                    out_vertices[out_count++] = intersect(prev, curr, edge);
                }
                out_vertices[out_count++] = curr;
            }
            else if (prev_inside) {
                out_vertices[out_count++] = intersect(prev, curr, edge);
            }

            prev = curr;
            prev_inside = curr_inside;
        }
        return out_count;
    }

    template<typename T, typename U>
    T Delaunay<T, U>::clipPolygonToRect(const Vec2<U>* vertices, T vertex_n, Vec2<U>* out_vertices) {
        static constexpr T kMax = 256;
        Vec2<U> buffer1[kMax];
        Vec2<U> buffer2[kMax];

        T n = vertex_n;
        std::memcpy((void*)buffer1, (void*)vertices, vertex_n * sizeof(Vec2<U>));

        n = clipAgainstEdge(buffer1, n, Edge::Left, buffer2);
        n = clipAgainstEdge(buffer2, n, Edge::Right, buffer1);
        n = clipAgainstEdge(buffer1, n, Edge::Top, buffer2);
        n = clipAgainstEdge(buffer2, n, Edge::Bottom, out_vertices);

        return n;
    }

    template<typename T, typename U>
    T Delaunay<T, U>::legalize(T a) {
        T i = 0;
        T ar = 0;
        edge_stack_.clear();

        // Recursion eliminated with a fixed-size stack
        while (true) {
            const T b = halfedges_[a];

            /*
             *  If the pair of triangles doesn't satisfy the Delaunay condition
             *  (p1 is inside the circumcircle of [p0, pl, pr]), flip them,
             *  then do the same check/flip recursively for the new pair of triangles
             *
             *           pl                    pl
             *          /||\                  /  \
             *       al/ || \bl            al/    \a
             *        /  ||  \              /      \
             *       /  a||b  \    flip    /___ar___\
             *     p0\   ||   /p1   =>   p0\---bl---/p1
             *        \  ||  /              \      /
             *       ar\ || /br             b\    /br
             *          \||/                  \  /
             *           pr                    pr
             */
            const T a0 = 3 * (a / 3);
            ar = a0 + (a + 2) % 3;

            if (b == kInvalidIndex) {
                if (i > 0) {
                    i--;
                    a = edge_stack_[i];
                    continue;
                }
                else {
                    //i = kInvalidIndex;
                    break;
                }
            }

            const T b0 = 3 * (b / 3);
            const T al = a0 + (a + 1) % 3;
            const T bl = b0 + (b + 2) % 3;

            const T p0 = triangle_indices_[ar];
            const T pr = triangle_indices_[a];
            const T pl = triangle_indices_[al];
            const T p1 = triangle_indices_[bl];

            const bool illegal = vertex_list_[p1].inCircle(vertex_list_[p0], vertex_list_[pr], vertex_list_[pl]);

            if (illegal) {
                triangle_indices_[a] = p1;
                triangle_indices_[b] = p0;

                auto hbl = halfedges_[bl];

                // Edge swapped on the other side of the hull (rare); fix the halfedge reference
                if (hbl == kInvalidIndex) {
                    T e = hull_start_;
                    do {
                        if (hull_tri_[e] == bl) {
                            hull_tri_[e] = a;
                            break;
                        }
                        e = hull_next_[e];
                    } while (e != hull_start_);
                }
                link(a, hbl);
                link(b, halfedges_[ar]);
                link(ar, bl);
                T br = b0 + (b + 1) % 3;

                if (i < edge_stack_.size()) {
                    edge_stack_[i] = br;
                }
                else {
                    edge_stack_.push(br);
                }
                i++;

            }
            else {
                if (i > 0) {
                    i--;
                    a = edge_stack_[i];
                    continue;
                }
                else {
                    break;
                }
            }
        }
        return ar;
    }

    template<typename T, typename U>
    inline T Delaunay<T, U>::hashKey(const Vec2<U>& pos) const {
        const Vec2<U> d = pos - center_;
        return fastMod(
            static_cast<T>(std::llround(std::floor(d.pseudoAngle() * static_cast<double>(hash_size_)))),
            hash_size_);
    }


    template<typename T, typename U>
    T Delaunay<T, U>::addTriangle(T i0, T i1, T i2, T a, T b, T c) {
        T t = static_cast<T>(triangle_indices_.size());
        triangle_indices_.push(i0);
        triangle_indices_.push(i1);
        triangle_indices_.push(i2);
        link(t, a);
        link(t + 1, b);
        link(t + 2, c);
        return t;
    }


    template<typename T, typename U>
    void Delaunay<T, U>::link(const T a, const T b) {
        T s = static_cast<T>(halfedges_.size());
        if (a == s) {
            halfedges_.push(b);
        }
        else if (a < s) {
            halfedges_[a] = b;
        }
        else {
            throw std::runtime_error("Cannot link edge");
        }
        if (b != kInvalidIndex) {
            T s2 = static_cast<T>(halfedges_.size());
            if (b == s2) {
                halfedges_.push(a);
            }
            else if (b < s2) {
                halfedges_[b] = a;
            }
            else {
                throw std::runtime_error("Cannot link edge");
            }
        }
    }

    template<typename T, typename U>
    inline bool Delaunay<T, U>::isClockwise(const Vec2<U>& a, const Vec2<U>& b, const Vec2<U>& c) {
        bool result = Vec2<U>::isClockwise(a, b, c);
        if (flip_y_ == true) {
            return !result;
        }
        return result;
    }

    template<typename T, typename U>
    inline int32_t Delaunay<T, U>::compareAngle(const void* a, const void* b) {
        U diff = static_cast<const AngleCenter*>(a)->angle - static_cast<const AngleCenter*>(b)->angle;
        return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
    }


    // @see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
    template<typename T, typename U>
    inline T Delaunay<T, U>::fastMod(const T i, const T c) {
        return i >= c ? i % c : i;
    }

    template<typename T, typename U>
    void Delaunay<T, U>::freeVoronoiCells() {
        if (voronoi_cells_) {
            for (auto i = 0; i < voronoi_cell_count_; i++) {
                free(voronoi_cells_[i]);  // Free each cell
            }
            free(voronoi_cells_);
            voronoi_cells_ = nullptr;
        }

        if (voronoi_cell_sizes_) {
            free(voronoi_cell_sizes_);
            voronoi_cell_sizes_ = nullptr;
        }
    }


    // Force code generation for these specializations
    template class Delaunay<int64_t, double>;
    template class Delaunay<int32_t, double>;
    template class Delaunay<int64_t, float>;
    template class Delaunay<int32_t, float>;

} // End of namespace Grain
