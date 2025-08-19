//
//  ValueGrid.hpp
//
//  Created by Roald Christesen on 07.05.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
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
        uint16_t m_main_version = 1;    ///< Main file version
        uint16_t m_sub_version = 0;     ///< Sub file version
        int16_t m_data_type = Type::kType_Undefined;    ///< Data type, one of `Type::kType` ...

        int32_t m_width{};              ///< Width, number of values in x-direction
        int32_t m_height{};             ///< Height, number of values in y-direction

        int32_t m_x_index{};            ///< Index in x-direction, useful for defining the position inside a grid of ValueGrids
        int32_t m_y_index{};            ///< Index in y-direction, useful for defining the position inside a grid of ValueGrids

        Flags m_feature_flags;          ///< Features included for this value grid, see `kFeature_...`

        // Feature data
        int32_t m_srid;                 ///< Spatial Reference System Identifier (SRID)
        RangeRectFix m_bbox;            ///< Bounding box for the region. If used in a geographic context, these coordinates must be in the SRID specified by `m_srid`

        // Value data
        T m_min_value{};                ///< Min value in the grid. Only valid after reading from file or after using updateMinMax
        T m_max_value{};                ///< Max value in the grid. Only valid after reading from file or after using updateMinMax

        int32_t m_value_count{};        ///< Number of values
        T* m_values = nullptr;          ///< Memory, where values are stored
        T m_invalid_value;              ///< Value to return, if request is out of range

    public:
        ValueGrid() noexcept : m_width(0), m_height(0) {
            m_value_count = 0;
            m_values = nullptr;
        }

        ValueGrid(int32_t width, int32_t height) noexcept : m_width(width), m_height(height) {
            if ((size_t)m_width * (size_t)m_height <= std::numeric_limits<int32_t>::max()) {
                m_value_count = width * height;
            }
            else {
                m_value_count = 0;
            }
            _initMem();
        }

        ~ValueGrid() noexcept {
            delete [] m_values;
        }

        const char* className() const noexcept override { return "ValueGrid"; }

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
            log << "data type: " << Type::typeName(valueDataType()) << ", version: " << m_main_version << "." << m_sub_version << std::endl;
            log << "dimension: " << m_width << " * " << m_height << std::endl;
            log++;
            if (hasFeature(kFeature_MinMax)) {
                log << "min: " << min() << ", max: " << max() << std::endl;
            }
            if (hasFeature(kFeature_GeoInfo)) {
                log << "Geo SRID: " << m_srid;
                log << ", bbox: " << m_bbox << std::endl;
            }
        }


        virtual const char* fileSignature() const noexcept { return "ValGrid_"; }


        bool _initMem() {
            if (m_values != nullptr) {
                delete [] m_values;
                m_values = nullptr;
            }
            if (m_value_count > 0) {
                m_values = new T[static_cast<size_t>(m_value_count)];
            }
            return m_values != nullptr;
        }

        bool _initMemThrow() {
            if (!_initMem()) {
                throw ErrorCode::MemCantAllocate;
            }
            else {
                return true;
            }
        }


        bool hasValues() const noexcept { return m_values != nullptr; }
        int16_t valueDataType() const noexcept;
        int32_t width() const noexcept { return m_width; }
        int32_t height() const noexcept { return m_height; }
        Rectd rect() const noexcept { return Rectd(m_width, m_height); }

        T min() const noexcept { return m_min_value; }
        T max() const noexcept { return m_max_value; }

        T minValueForType() const noexcept {
            return std::numeric_limits<T>::lowest();
        }

        T maxValueForType() const noexcept {
            return std::numeric_limits<T>::max();
        }

        const T* ptrForRow(int32_t y) const noexcept {
            if (_canAccessXY(0, y)) {
                return &m_values[_indexForXY(0, y)];
            }
            else {
                return nullptr;
            }
        }

        const T* ptrAtXY(int32_t x, int32_t y) const noexcept {
            if (_canAccessXY(x, y)) {
                return &m_values[_indexForXY(x, y)];
            }
            else {
                return nullptr;
            }
        }

        T* mutPtrForRow(int32_t y) noexcept { return (T*)ptrForRow(y); }
        T* mutPtrAtXY(int32_t x, int32_t y) const noexcept { return (T*)ptrAtXY(x, y); }


        void setXIndex(int32_t x_index) noexcept {
            m_x_index = x_index;
        }

        void setYIndex(int32_t y_index) noexcept {
            m_x_index = y_index;
        }

        void setMinMax(T min, T max) noexcept {
            m_min_value = min;
            m_max_value = max;
            setFeature(kFeature_MinMax);
        }

        void updateMinMax() noexcept {
            if (m_values != nullptr && m_value_count > 0) {
                m_min_value = maxValueForType();
                m_max_value = minValueForType();
                bool check_invalid = hasFeature(kFeature_Invalid_Value);
                if (check_invalid) {
                    for (int32_t i = 0; i < m_value_count; i++) {
                        T v = m_values[i];
                        if (v != m_invalid_value) {
                            if (v < m_min_value) {
                                m_min_value = v;
                            }
                            if (v > m_max_value) {
                                m_max_value = v;
                            }
                        }
                    }
                }
                else {
                    for (int32_t i = 0; i < m_value_count; i++) {
                        T v = m_values[i];
                        if (v < m_min_value) {
                            m_min_value = v;
                        }
                        if (v > m_max_value) {
                            m_max_value = v;
                        }
                    }
                }
            }
            setFeature(kFeature_MinMax);
        }

        void setInvalidValue(T value) noexcept {
            m_invalid_value = value;
            m_feature_flags.setFlag(kFeature_Invalid_Value);
        }

        void setInvalidValueDefault() noexcept {
            m_invalid_value = minValueForType();
            m_feature_flags.setFlag(kFeature_Invalid_Value);
        }


        void setGeoInfo(int32_t srid, const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept;
        void setGeoInfo(int32_t srid, const RangeRectFix& bbox) noexcept;
        void setGeoInfo(int32_t srid, const RangeRectd& bbox) noexcept;

        T valueAtXY(int32_t x, int32_t y) noexcept {
            if (_canAccessXY(x, y)) {
                return m_values[_indexForXY(x, y)];
            }
            else {
                return 0;
            }
        }

        bool setValueAtXY(int32_t x, int32_t y, T value) noexcept {
            if (_canAccessXY(x, y)) {
                int32_t index = _indexForXY(x, y);
                T old_value = m_values[index];
                m_values[index] = value;
                return value != old_value;
            }
            else {
                return false;
            }
        }

        bool invalidateValueAtXY(int32_t x, int32_t y) noexcept {
            return setValueAtXY(x, y, m_invalid_value);
        }

        int32_t countInvalidValues() const noexcept {
            int32_t result = 0;
            if (hasFeature(kFeature_Invalid_Value) && m_values != nullptr) {
                for (int32_t i = 0; i < m_value_count; i++) {
                    if (m_values[i] == m_invalid_value) {
                        result++;
                    }
                }
            }
            return result;
        }

        // Feature flags
        void setFeature(int32_t index) noexcept { m_feature_flags.setFlag(index); }
        void clearFeature(int32_t index) noexcept { m_feature_flags.clearFlag(index); }
        bool hasFeature(int32_t index) const noexcept { return m_feature_flags.isSet(index); }


        inline bool _canAccessXY(int32_t x, int32_t y) const noexcept {
            return x >= 0 && x < m_width && y >= 0 && y < m_height && m_values != nullptr;
        }

        inline bool _validXY(int32_t x, int32_t y) const noexcept {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        inline int32_t _indexForXY(int32_t x, int32_t y) const noexcept {
            return y * m_width + x;
        }

        void clear(T value) noexcept {
            if (m_values != nullptr) {
                for (int32_t i = 0; i < m_value_count; i++) {
                    m_values[i] = value;
                }
            }
        }

        /**
         *  @brief Marks all values in the grid as invalid.
         *
         *  This function sets all values in the grid to an appropriate invalid state.
         *  Ensure that an appropriate value has been set for the invalid state by calling the method `setInvalidValue()` beforehand.
         */
        void invalidate() noexcept { clear(m_invalid_value); }


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

        Image* buildImage(bool flip_y = false) const noexcept;
        Image* buildImageAlphaWhereUndefined(T undefined_value, bool flip_y = false) const noexcept;
    };


    // Standard types
    using ValueGridu8 = ValueGrid<uint8_t>; ///< 8 bit unsigned integer
    using ValueGridi = ValueGrid<int32_t>;  ///< 32 bit integer
    using ValueGridl = ValueGrid<int64_t>;  ///< 64 bit integer
    using ValueGridf = ValueGrid<float>;    ///< 32 bit floating point
    using ValueGridd = ValueGrid<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainValueGrid_hpp
