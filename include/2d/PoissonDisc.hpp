//
//  PoissonDisc.hpp
//
//  Created by Roald Christesen on from 01.06.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainPoissonDisc_hpp
#define GrainPoissonDisc_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "2d/RangeRect.hpp"

#include <vector>
#include <random>
#include <cmath>
#include <optional>


namespace Grain {

class PoissonDiscSampler;
class Image;
class ImageAccess;


typedef double (*PoissonDiscRandomFunc)(void* ref);


class PoissonDiscDensity {
protected:
    PoissonDiscSampler* sampler_ = nullptr;
    double min_radius_{};
    double max_radius_{};
    double radius_delta_{};

public:
    PoissonDiscDensity() = default;
    virtual ~PoissonDiscDensity() = default;

    virtual void setSampler(PoissonDiscSampler* sampler);
    virtual double densityAtPos(const Vec2d& pos);
};


class PoissonDiscImageDensity : public PoissonDiscDensity {
protected:
    Image* image_ = nullptr;
    ImageAccess* ia_ = nullptr;
    float ia_pixel_[4];
    Vec2d scale_ = { 1.0, 1.0 };

public:
    PoissonDiscImageDensity(Image* image);
    virtual ~PoissonDiscImageDensity();

    void setSampler(PoissonDiscSampler* sampler) override;
    void setImage(Image* image);
    double densityAtPos(const Vec2d& pos) override;
};


class PoissonDiscSampler {
protected:
    double min_x_, min_y_, max_x_, max_y_;
    double min_radius_, max_radius_;
    double cell_size_;
    int32_t k_, grid_width_, grid_height_;
    PoissonDiscDensity* density_;
    PoissonDiscRandomFunc random_func_;

    std::vector<std::optional<Vec2d>> grid_;
    std::vector<Vec2d> point_queue_;
    bool first_point_flag_ = true;

public:
    PoissonDiscSampler(const RangeRectd range, PoissonDiscDensity* density, double r_min, double r_max, int32_t max_tries = 30);

    ~PoissonDiscSampler() {
    }


    double width() { return max_x_ - min_x_; }
    double height() { return max_y_ - min_y_; }
    double minRadius() { return min_radius_; }
    double maxRadius() { return max_radius_; }

    void reset();
    std::optional<Vec2d> next();
    std::vector<Vec2d> all();
    bool done() const;

private:
    double distSquared(double x1, double y1, double x2, double y2) const;
    bool isValidPoint(double x, double y) const;
    Vec2d createNewPoint(double x, double y);

    static double _randomFunc(void* ref) { return static_cast<double>(rand()) / RAND_MAX; }
};

} // End of namespace Grain

#endif //GrainPoissonDisc_hpp