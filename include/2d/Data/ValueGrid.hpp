//
//  ValueGrid.hpp
//
//  Created by Roald Christesen on 07.05.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainValueGrid_hpp
#define GrainValueGrid_hpp

#include "Type/Object.hpp"
#include "Type/Type.hpp"
#include "String/String.hpp"
#include "Core/Log.hpp"
#include "2d/RangeRect.hpp"
#include "Type/Flags.hpp"
#include "File/File.hpp"


namespace Grain {

class Image;


/**
 *  @brief Class for storing values in a 2-dimensional array (grid).
 *
 *  A Value Grid is an organized set of values in row/column order.
 *  The class can be used to store simple data types as well as combined data, such as 2D and 3D vectors."
 *
 *  Features:
 *  - The width and height must each be greater than 0, with a maximum total number of values equal to 2^31 - 1 (2,147,483,647).
 *  - Uses a specified data type for storing the values.
 *  - Provides methods for accessing the individual values by x and y coordinates.
 *  - Provides methods for writing to a file and reading from a file.
 *  - Can contain some feature information, such as data about the grid (e.g., geo information).
 */
template <class T>
class ValueGrid : public Object {

public:
    enum {
        kFeature_MinMax = 0,        ///< Min/max feature id
        kFeature_Invalid_Value = 1, ///< Invalid value is used to mark values as invalid
        kFeature_GeoInfo = 2,       ///< Geo information feature id
        kFeature_CustomInfo = 31    ///< Custom information feature id
    };

    enum {
        kSignatureLength = 8,       ///< Length of file signature in bytes
        kCRSStringLength = 16       ///< Length of CRS string in bytes
    };

protected:
    uint16_t main_version_ = 1;     ///< Main file version
    uint16_t sub_version_ = 0;      ///< Sub file version
    int16_t data_type_ = Type::kType_Undefined;    ///< Data type, one of `Type::kType` ...

    int32_t width_{};               ///< Width, number of values in x-direction
    int32_t height_{};              ///< Height, number of values in y-direction
    int32_t x_index_{};             ///< Index in x-direction, useful for defining the position inside a grid of ValueGrids
    int32_t y_index_{};             ///< Index in y-direction, useful for defining the position inside a grid of ValueGrids

    Flags feature_flags_{};         ///< Features included for this value grid, see `kFeature_...`

    // Feature data
    int32_t srid_{};                ///< Spatial Reference System Identifier (SRID)
    RangeRectFix bbox_{};           ///< Bounding box for the region. If used in a geographic context, these coordinates must be in the SRID specified by `srid_`

    // Value data
    T min_value_{};                 ///< Min value in the grid. Only valid after reading from file or after using updateMinMax
    T max_value_{};                 ///< Max value in the grid. Only valid after reading from file or after using updateMinMax

    int32_t value_count_{};         ///< Number of values
    T* values_ = nullptr;           ///< Memory, where values are stored
    T invalid_value_{};             ///< Value to return, if request is println of range

public:
    ValueGrid() noexcept {
        value_count_ = 0;
        values_ = nullptr;
    }

    ValueGrid(int32_t width, int32_t height) noexcept : width_(width), height_(height) {
        if ((size_t)width_ * (size_t)height_ <= std::numeric_limits<int32_t>::max()) {
            value_count_ = width * height;
        }
        else {
            value_count_ = 0;
        }
        _initMem();
    }

    ~ValueGrid() noexcept override {
        delete [] values_;
    }

    [[nodiscard]] const char* className() const noexcept override {
        return "ValueGrid";
    }

    friend std::ostream& operator << (std::ostream& os, const ValueGrid* o) {
        o == nullptr ? os << "ValueGrid nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const ValueGrid& o) {
        o.log(os, 0, o.className());
        return os;
    }

    void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
        Log log(os, indent);
        log.header(label);
        log << "data type: " << Type::typeName(valueDataType()) << ", version: " << main_version_ << "." << sub_version_ << std::endl;
        log << "dimension: " << width_ << " * " << height_ << std::endl;
        log++;
        if (hasFeature(kFeature_MinMax)) {
            log << "min: " << min() << ", max: " << max() << std::endl;
        }
        if (hasFeature(kFeature_GeoInfo)) {
            log << "Geo SRID: " << srid_;
            log << ", bbox: " << bbox_ << std::endl;
        }
    }


    [[nodiscard]] virtual const char* fileSignature() const noexcept { return "ValGrid_"; }


    bool _initMem() {
        if (values_ != nullptr) {
            delete [] values_;
            values_ = nullptr;
        }
        if (value_count_ > 0) {
            values_ = new T[static_cast<size_t>(value_count_)];
        }
        return values_ != nullptr;
    }

    bool _initMemThrow() {
        if (!_initMem()) {
            throw ErrorCode::MemCantAllocate;
        }
        else {
            return true;
        }
    }


    [[nodiscard]] bool hasValues() const noexcept { return values_ != nullptr; }
    [[nodiscard]] int16_t valueDataType() const noexcept;
    [[nodiscard]] int32_t width() const noexcept { return width_; }
    [[nodiscard]] int32_t height() const noexcept { return height_; }
    [[nodiscard]] Rectd rect() const noexcept { return Rectd(width_, height_); }

    T min() const noexcept { return min_value_; }
    T max() const noexcept { return max_value_; }

    T minValueForType() const noexcept {
        return std::numeric_limits<T>::lowest();
    }

    T maxValueForType() const noexcept {
        return std::numeric_limits<T>::max();
    }

    const T* ptrForRow(int32_t y) const noexcept {
        if (_canAccessXY(0, y)) {
            return &values_[_indexForXY(0, y)];
        }
        else {
            return nullptr;
        }
    }

    const T* ptrAtXY(int32_t x, int32_t y) const noexcept {
        if (_canAccessXY(x, y)) {
            return &values_[_indexForXY(x, y)];
        }
        else {
            return nullptr;
        }
    }

    T* mutPtrForRow(int32_t y) noexcept { return (T*)ptrForRow(y); }
    T* mutPtrAtXY(int32_t x, int32_t y) const noexcept { return (T*)ptrAtXY(x, y); }


    void setXIndex(int32_t x_index) noexcept {
        x_index_ = x_index;
    }

    void setYIndex(int32_t y_index) noexcept {
        y_index_ = y_index;
    }

    void setMinMax(T min, T max) noexcept {
        min_value_ = min;
        max_value_ = max;
        setFeature(kFeature_MinMax);
    }

    void updateMinMax() noexcept {
        if (values_ != nullptr && value_count_ > 0) {
            min_value_ = maxValueForType();
            max_value_ = minValueForType();
            bool check_invalid = hasFeature(kFeature_Invalid_Value);
            if (check_invalid) {
                for (int32_t i = 0; i < value_count_; i++) {
                    T v = values_[i];
                    if (v != invalid_value_) {
                        if (v < min_value_) {
                            min_value_ = v;
                        }
                        if (v > max_value_) {
                            max_value_ = v;
                        }
                    }
                }
            }
            else {
                for (int32_t i = 0; i < value_count_; i++) {
                    T v = values_[i];
                    if (v < min_value_) {
                        min_value_ = v;
                    }
                    if (v > max_value_) {
                        max_value_ = v;
                    }
                }
            }
        }
        setFeature(kFeature_MinMax);
    }

    void setInvalidValue(T value) noexcept {
        invalid_value_ = value;
        feature_flags_.setFlag(kFeature_Invalid_Value);
    }

    void setInvalidValueDefault() noexcept {
        invalid_value_ = minValueForType();
        feature_flags_.setFlag(kFeature_Invalid_Value);
    }


    void setGeoInfo(int32_t srid, const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept;
    void setGeoInfo(int32_t srid, const RangeRectFix& bbox) noexcept;
    void setGeoInfo(int32_t srid, const RangeRectd& bbox) noexcept;

    T valueAtXY(int32_t x, int32_t y) noexcept {
        if (_canAccessXY(x, y)) {
            return values_[_indexForXY(x, y)];
        }
        else {
            return 0;
        }
    }

    bool setValueAtXY(int32_t x, int32_t y, T value) noexcept {
        if (_canAccessXY(x, y)) {
            int32_t index = _indexForXY(x, y);
            T old_value = values_[index];
            values_[index] = value;
            return value != old_value;
        }
        else {
            return false;
        }
    }

    bool invalidateValueAtXY(int32_t x, int32_t y) noexcept {
        return setValueAtXY(x, y, invalid_value_);
    }

    [[nodiscard]] int32_t countInvalidValues() const noexcept {
        int32_t result = 0;
        if (hasFeature(kFeature_Invalid_Value) && values_ != nullptr) {
            for (int32_t i = 0; i < value_count_; i++) {
                if (values_[i] == invalid_value_) {
                    result++;
                }
            }
        }
        return result;
    }

    // Feature flags
    void setFeature(int32_t index) noexcept { feature_flags_.setFlag(index); }
    void clearFeature(int32_t index) noexcept { feature_flags_.clearFlag(index); }
    [[nodiscard]] bool hasFeature(int32_t index) const noexcept { return feature_flags_.isSet(index); }


    [[nodiscard]] inline bool _canAccessXY(int32_t x, int32_t y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_ && values_ != nullptr;
    }

    [[nodiscard]] inline bool _validXY(int32_t x, int32_t y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    [[nodiscard]] inline int32_t _indexForXY(int32_t x, int32_t y) const noexcept {
        return y * width_ + x;
    }

    void clear(T value) noexcept {
        if (values_ != nullptr) {
            for (int32_t i = 0; i < value_count_; i++) {
                values_[i] = value;
            }
        }
    }

    /**
     *  @brief Marks all values in the grid as invalid.
     *
     *  This function sets all values in the grid to an appropriate invalid state.
     *  Ensure that an appropriate value has been set for the invalid state by calling the method `setInvalidValue()` beforehand.
     */
    void invalidate() noexcept { clear(invalid_value_); }


    ErrorCode fourToOne(ValueGrid* src_grid[4], uint8_t mask) noexcept;
    ErrorCode fillMipmapQuadrant(const ValueGrid* value_grid, int32_t quadrant_index) noexcept;

    ErrorCode writeFile(const String& file_path) noexcept;
    ErrorCode readFile(const String& file_path) noexcept;

    virtual void writeCustomInfo() {};      ///< Can be overriden by derivated classes.
    virtual void readCustomInfo() {};       ///< Can be overriden by derivated classes.

    void _writeDataToFile(File* file);
    void _writeTypeValue(File* file, T value);


    void _readDataFromFile(File* file);
    T _readTypeValue(File* file);

    ErrorCode writeCVF2File(const String& file_path, LengthUnit length_unit, int32_t min_digits, int32_t max_digits) noexcept;

    [[nodiscard]] Image* buildImage(bool flip_y = false) const noexcept;
    [[nodiscard]] Image* buildImageAlphaWhereUndefined(T undefined_value, bool flip_y = false) const noexcept;
};


// Standard types
using ValueGridu8 = ValueGrid<uint8_t>; ///< 8 bit unsigned integer
using ValueGridi = ValueGrid<int32_t>;  ///< 32 bit integer
using ValueGridl = ValueGrid<int64_t>;  ///< 64 bit integer
using ValueGridf = ValueGrid<float>;    ///< 32 bit floating point
using ValueGridd = ValueGrid<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainValueGrid_hpp
