//
//  PoissonDisc.cpp
//
//  Created by Roald Christesen on from 01.06.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/PoissonDisc.hpp"
#include "Image/Image.hpp"
#include "Color/RGB.hpp"


namespace Grain {

void PoissonDiscDensity::setSampler(PoissonDiscSampler* sampler) {
    sampler_ = sampler;
    min_radius_ = sampler->minRadius();
    max_radius_ = sampler->maxRadius();
    radius_delta_ = max_radius_ - min_radius_;
}


double PoissonDiscDensity::densityAtPos(const Vec2d &pos) {
    return 0;
}


PoissonDiscImageDensity::PoissonDiscImageDensity(Image* image) {
    setImage(image);
}


PoissonDiscImageDensity::~PoissonDiscImageDensity() {
    GRAIN_RELEASE(image_);
    delete ia_;
}


void PoissonDiscImageDensity::setSampler(PoissonDiscSampler* sampler) {
    PoissonDiscDensity::setSampler(sampler);
    scale_.x_ = image_->width() / sampler->width();
    scale_.y_ = image_->height() / sampler->height();
}


void PoissonDiscImageDensity::setImage(Image* image) {
    if (image != image_) {
        if (image_ != nullptr) {
            GRAIN_RELEASE(image_);
            image_ = nullptr;
        }

        if (image != nullptr) {
            image_ = image;
            GRAIN_RETAIN(image_);
            if (ia_ != nullptr) {
                delete ia_;
                ia_ = nullptr;
            }
            ia_ = new ImageAccess(image_, ia_pixel_);
        }
    }

    scale_.set(1.0, 1.0);
}


double PoissonDiscImageDensity::densityAtPos(const Vec2d& pos) {
    ia_->readInterpolated(pos * scale_);
    RGB rgb(ia_pixel_);
    double brightness = rgb.lumina709();
    return min_radius_ + brightness * brightness * radius_delta_;
}


PoissonDiscSampler::PoissonDiscSampler(
    const RangeRectd range,
    PoissonDiscDensity* density,
    double r_min,
    double r_max,
    int32_t max_tries)
{
    min_x_ = range.min_x_;
    min_y_ = range.min_y_;
    max_x_ = range.max_x_;
    max_y_ = range.max_y_;

    density_ = density;

    min_radius_ = std::max(r_min, 1.0);
    max_radius_ = r_max;

    k_ = std::max(max_tries, 2);
    random_func_ = _randomFunc;

    cell_size_ = min_radius_ / std::sqrt(2.0);
    grid_width_ = static_cast<int32_t>(std::ceil((max_x_ - min_x_) / cell_size_));
    grid_height_ = static_cast<int32_t>(std::ceil((max_y_ - min_y_) / cell_size_));
    grid_.assign(grid_width_ * grid_height_, std::nullopt);

    std::cout << "min_x_: " << min_x_ << std::endl;
    std::cout << "min_y_: " << min_y_ << std::endl;
    std::cout << "max_x_: " << max_x_ << std::endl;
    std::cout << "max_y_: " << max_y_ << std::endl;
    std::cout << "min_radius_: " << min_radius_ << std::endl;
    std::cout << "max_radius_: " << max_radius_ << std::endl;
    std::cout << "k_: " << k_ << std::endl;
    std::cout << "cell_size_: " << cell_size_ << std::endl;
    std::cout << "grid_width_: " << grid_width_ << std::endl;
    std::cout << "grid_height_: " << grid_height_ << std::endl;

    reset();

    density->setSampler(this);
}


void PoissonDiscSampler::reset() {
    point_queue_.clear();
    grid_.assign(grid_width_ * grid_height_, std::nullopt);
    first_point_flag_ = true;
}


std::optional<Vec2d> PoissonDiscSampler::next() {
    double x, y;

    if (first_point_flag_ == true) {
        first_point_flag_ = false;
        x = min_x_ + (max_x_ - min_x_) * random_func_(this);
        y = min_y_ + (max_y_ - min_y_) * random_func_(this);
        x = std::clamp<double>(x, 0, max_x_ - 1);
        y = std::clamp<double>(y, 0, max_y_ - 1);
        std::cout << "x: " << x << ", y: " << y << std::endl;
        return createNewPoint(x, y);
    }

    while (!point_queue_.empty()) {
        int idx = static_cast<int>(random_func_(this) * point_queue_.size());
        const Vec2d& base = point_queue_[idx];
        double base_radius = density_->densityAtPos(base);

        for (int i = 0; i < k_; ++i) {
            double distance = base_radius * (random_func_(this) + 1.0);
            double angle = 2.0 * M_PI * random_func_(this);
            x = base.x_ + distance * std::cos(angle);
            y = base.y_ + distance * std::sin(angle);
            if (isValidPoint(x, y)) {
                return createNewPoint(x, y);
            }
        }
        point_queue_.erase(point_queue_.begin() + idx);
    }
    return std::nullopt;
}


std::vector<Vec2d> PoissonDiscSampler::all() {
    reset();
    std::vector<Vec2d> results;
    std::optional<Vec2d> pt;
    while ((pt = next())) {
        results.push_back(*pt);
    }
    return results;
}


bool PoissonDiscSampler::done() const {
    return first_point_flag_ == false && point_queue_.empty();
}


double PoissonDiscSampler::distSquared(double x1, double y1, double x2, double y2) const {
    double dx = x2 - x1, dy = y2 - y1;
    return dx * dx + dy * dy;
}


bool PoissonDiscSampler::isValidPoint(double x, double y) const {
    if (x < min_x_ || x >= max_x_ || y < min_y_ || y >= max_y_) {
        return false;
    }

    double r = density_->densityAtPos(Vec2d(x, y));
    int col = static_cast<int>((x - min_x_) / cell_size_);
    int row = static_cast<int>((y - min_y_) / cell_size_);

    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            int nx = col + dx, ny = row + dy;
            if (nx >= 0 && nx < grid_width_ && ny >= 0 && ny < grid_height_) {
                auto& cell = grid_[nx + ny * grid_width_];
                if (cell && distSquared(x, y, cell->x_, cell->y_) <= r * r) {
                    return false;
                }
            }
        }
    }

    return true;
}


Vec2d PoissonDiscSampler::createNewPoint(double x, double y) {
    Vec2d p = { x, y };
    int32_t col = static_cast<int32_t>((x - min_x_) / cell_size_);
    int32_t row = static_cast<int32_t>((y - min_y_) / cell_size_);
    grid_[col + row * grid_width_] = p;
    point_queue_.push_back(p);
    return p;
}

}  // End of namespace Grain