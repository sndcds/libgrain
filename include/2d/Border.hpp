//
//  Border.hpp
//
//  Created by Roald Christesen on from 24.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainBorder_hpp
#define GrainBorder_hpp

namespace Grain {

template <class T>
class Border {

public:
    T top_{};
    T right_{};
    T bottom_{};
    T left_{};

public:
    Border() noexcept = default;

    Border(T top, T right, T bottom, T left) noexcept :
        top_(top), right_(right), bottom_(bottom), left_(left) {}

    explicit Border(T horizontal, T vertical) noexcept :
        top_(vertical), right_(horizontal), bottom_(vertical), left_(horizontal) {}

    explicit Border(T size) noexcept :
        top_(size), right_(size), bottom_(size), left_(size) {}

    virtual ~Border() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Border";
    }

    friend std::ostream& operator << (std::ostream& os, const Border* o) {
        o == nullptr ? os << "Border nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Border& o) {
        os << o.top_ << ", " << o.right_ << ", " << o.bottom_ << ", " << o.left_;
        return os;
    }


    [[nodiscard]] T left() const noexcept { return left_; }
    [[nodiscard]] T right() const noexcept { return right_; }
    [[nodiscard]] T top() const noexcept { return top_; }
    [[nodiscard]] T bottom() const noexcept { return bottom_; }
    [[nodiscard]] T width() const noexcept { return left_ + right_; }
    [[nodiscard]] T height() const noexcept { return top_ + bottom_; }

    void set(T size) noexcept {
        top_ = right_ = bottom_ = left_ = size;
    }

    void set(T vertical, T horizontal) noexcept {
        top_ = bottom_ = vertical;
        right_ = left_ = horizontal;
    }

    void set(T top, T right, T bottom, T left) noexcept {
        top_ = top;
        right_ = right;
        bottom_ = bottom;
        left_ = left;
    }

    bool set(T* values, int32_t n) noexcept {
        if (values != nullptr) {
            if (n == 1) {
                top_ = right_ = bottom_ = left_ = values[0];
                return true;
            }
            if (n == 2) {
                top_ = bottom_ = values[0];
                right_ = left_ = values[1];
                return true;
            }
            if (n == 4) {
                top_ = values[0];
                right_ = values[1];
                bottom_ = values[2];
                left_ = values[3];
                return true;
            }
        }
        return false;
    }
};


// Standard types
using Borderi = Border<int32_t>;    ///< 32 bit integer
using Borderl = Border<int64_t>;    ///< 64 bit integer
using Borderf = Border<float>;      ///< 32 bit floating point
using Borderd = Border<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainBorder_hpp
