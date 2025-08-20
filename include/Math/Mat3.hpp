//
//  Mat3.hpp
//
//  Created by Roald Christesen on 04.10.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainMat3_hpp
#define GrainMat3_hpp

#include "Grain.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"


namespace Grain {

    /**
     *  @brief 3x3 Matrix, row-major order.
     *
     *  `Mat3` represents 3x3 matrices, which are essential tools for various
     *  applications involving 3D transformations and computations. his templated
     *  class supports diverse data types, allowing versatility in numeric
     *  representation. Predefined specialized versions include datatypes float and
     *  double, referred to as `Mat3f` and `Mat3d` respectively.
     *
     *  `Mat3` is indispensable for tasks like 3D graphics transformations,
     *  coordinate rotations, and scaling operations. It finds applications in
     *  computer graphics, computer vision, robotics, and simulations. `Mat3`
     *  empowers you to efficiently perform complex spatial transformations,
     *  offering accuracy and flexibility in calculations.
     */
    template <ScalarType T>
    class Mat3 {
    public:
        Mat3() { identity(); }
        constexpr Mat3(T v00, T v01, T v02, T v10, T v11, T v12, T v20, T v21, T v22) noexcept {
            T* d = mutDataPtr();
            d[0] = v00; d[1] = v01; d[2] = v02;
            d[3] = v10; d[4] = v11; d[5] = v12;
            d[6] = v20; d[7] = v21; d[8] = v22;
        }
        Mat3(T* v, bool row_order = true) noexcept { set(v, row_order); }

        [[nodiscard]] virtual const char* className() const noexcept { return "Mat3"; }

        friend std::ostream& operator << (std::ostream& os, const Mat3* o) {
            o == nullptr ? os << "Mat3 nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Mat3& o) {
            const T* d = o.dataPtr();
            os << d[0] << ", " << d[1] << ", " << d[2] << " | ";
            os << d[3] << ", " << d[4] << ", " << d[5] << " | ";
            os << d[6] << ", " << d[7] << ", " << d[8];
            return os;
        }


        Mat3& operator *= (const Mat3& other) { mul(other, *this); return *this; }

        T* mutDataPtr() noexcept { return &m_data[0][0]; }
        const T* dataPtr() const noexcept { return &m_data[0][0]; }

        T valueAtRowColumn(int32_t row, int32_t column) const noexcept {
            if (row >= 0 && row < 3 && column >= 0 && column < 3) {
                return m_data[row][column];
            }
            else {
                return 0;
            }
        }

        void set(T v00, T v01, T v02, T v10, T v11, T v12, T v20, T v21, T v22) noexcept {
            T* d = mutDataPtr();
            d[0] = v00; d[1] = v01; d[2] = v02;
            d[3] = v10; d[4] = v11; d[5] = v12;
            d[6] = v20; d[7] = v21; d[8] = v22;
        }

        void set(T* v, bool row_order = true) noexcept {
            if (v) {
                T* d = mutDataPtr();
                if (row_order) {
                    d[0] = v[0]; d[1] = v[1]; d[2] = v[2];
                    d[3] = v[3]; d[4] = v[4]; d[5] = v[5];
                    d[6] = v[6]; d[7] = v[7]; d[8] = v[8];
                }
                else {
                    d[0] = v[0]; d[3] = v[1]; d[6] = v[2];
                    d[1] = v[3]; d[4] = v[4]; d[7] = v[5];
                    d[2] = v[6]; d[5] = v[7]; d[8] = v[8];
                }
            }
        }

        void clear() noexcept {
            T* d = mutDataPtr();
            for (int i = 0; i < 9; i++) {
                *d++ = 0;
            }
        }

        void identity() noexcept {
            T* d = mutDataPtr();
            d[0] = 1; d[1] = 0; d[2] = 0;
            d[3] = 0; d[4] = 1; d[5] = 0;
            d[6] = 0; d[7] = 0; d[8] = 1;
        }


        void translateX(T t) noexcept { translate(t, 0); }
        void translateY(T t) noexcept { translate(0, t); }

        void translate(T tx, T ty) noexcept {
            Mat3<T> m(1, 0, tx, 0, 1, ty, 0, 0, 1);
            mul(m);
        }

        void rotateRad(T rad) noexcept {
            T c = std::cos(rad);
            T s = std::sin(rad);
            Mat3<T> m(c, -s, 0, s, c, 0, 0, 0, 1);
            mul(m);
        }

        void rotate(T deg) noexcept {
            rotateRad(deg * std::numbers::pi / 180.0);
        }

        void scale(T s) noexcept {
            Mat3<T> m(s, 0, 0, 0, s, 0, 0, 0, 1);
            mul(m);
        }

        void scale(T sx, T sy) noexcept {
            Mat3<T> m(sx, 0, 0, 0, sy, 0, 0, 0, 1);
            mul(m);
        }

        void transpose() noexcept {
            T* d = mutDataPtr();
            T temp;
            temp = d[1]; d[1] = d[3]; d[3] = temp;
            temp = d[2]; d[2] = d[6]; d[6] = temp;
            temp = d[5]; d[5] = d[7]; d[7] = temp;
        }


        T determinant() noexcept {
            T* d = mutDataPtr();
            return
                    d[0] * (d[4] * d[8] - d[5] * d[7]) -
                    d[1] * (d[3] * d[8] - d[5] * d[6]) +
                    d[2] * (d[3] * d[7] - d[4] * d[6]);
        }

        bool invert() noexcept { return invert(*this); }

        bool invert(Mat3& out_matrix) const noexcept {

            const T* s = dataPtr();
            T* d = out_matrix.dataPtr();
            T temp[9];

            temp[0] = s[4] * s[8] - s[5] * s[7];
            temp[1] = s[2] * s[7] - s[1] * s[8];
            temp[2] = s[1] * s[5] - s[2] * s[4];
            temp[3] = s[5] * s[6] - s[3] * s[8];
            temp[4] = s[0] * s[8] - s[2] * s[6];
            temp[5] = s[2] * s[3] - s[0] * s[5];
            temp[6] = s[3] * s[7] - s[4] * s[6];
            temp[7] = s[1] * s[6] - s[0] * s[7];
            temp[8] = s[0] * s[4] - s[1] * s[3];

            // Check determinant if it is 0
            T det = s[0] * temp[0] + s[1] * temp[3] + s[2] * temp[6];
            if (std::fabs(det) < std::numeric_limits<double>::epsilon()) {
                out_matrix.identity();
                return false;
            }

            // Divide by the determinant
            T inv_det = 1.0 / det;
            d[0] = inv_det * temp[0];
            d[1] = inv_det * temp[1];
            d[2] = inv_det * temp[2];
            d[3] = inv_det * temp[3];
            d[4] = inv_det * temp[4];
            d[5] = inv_det * temp[5];
            d[6] = inv_det * temp[6];
            d[7] = inv_det * temp[7];
            d[8] = inv_det * temp[8];

            return true;
        }

        void mul(const Mat3& m) noexcept {
            T* a = mutDataPtr();
            const T* b = m.dataPtr();
            T temp[9];
            temp[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
            temp[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
            temp[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
            temp[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
            temp[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
            temp[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
            temp[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
            temp[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
            temp[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
            memcpy(a, temp, 9 * sizeof(T));
        }

        void mul(const Mat3& m, Mat3& outMatrix) const noexcept {
            const T* a = dataPtr();
            const T* b = m.dataPtr();
            T temp[9];
            temp[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
            temp[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
            temp[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
            temp[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
            temp[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
            temp[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
            temp[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
            temp[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
            temp[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
            memcpy(outMatrix.mutDataPtr(), temp, 9 * sizeof(T));
        }

        void transformVec2(Vec2<T>& v) const noexcept {
            const T* s = dataPtr();
            T x = s[0] * v.m_x + s[1] * v.m_y + s[2];
            T y = s[3] * v.m_x + s[4] * v.m_y + s[5];
            v.m_x = x;
            v.m_y = y;
        }

        void transform2(T* values) const noexcept {
            if (values) {
                const T* s = dataPtr();
                T x = s[0] * values[0] + s[1] * values[1] + s[2];
                T y = s[3] * values[0] + s[4] * values[1] + s[5];
                values[0] = x;
                values[1] = y;
            }
        }

        void transform2(T* values, T* out_values) const noexcept {
            if (values && out_values) {
                const T* s = dataPtr();
                T x = s[0] * values[0] + s[1] * values[1] + s[2];
                T y = s[3] * values[0] + s[4] * values[1] + s[5];
                out_values[0] = x;
                out_values[1] = y;
            }
        }

        void transformVec3(Vec3<T>& v) const noexcept {
            const T* s = dataPtr();
            T x = s[0] * v.m_x + s[1] * v.m_y + s[2] * v.m_z;
            T y = s[3] * v.m_x + s[4] * v.m_y + s[5] * v.m_z;
            T z = s[6] * v.m_x + s[7] * v.m_y + s[8] * v.m_z;
            v.m_x = x;
            v.m_y = y;
            v.m_z = z;
        }

        void transform3(T* values) const noexcept {
            if (values) {
                const T* s = dataPtr();
                T v1 = s[0] * values[0] + s[1] * values[1] + s[2] * values[2];
                T v2 = s[3] * values[0] + s[4] * values[1] + s[5] * values[2];
                T v3 = s[6] * values[0] + s[7] * values[1] + s[8] * values[2];
                values[0] = v1;
                values[1] = v2;
                values[2] = v3;
            }
        }

        void transform3(const T* values, T* out_values) const noexcept {
            if (values && out_values) {
                const T* s = dataPtr();
                T v1 = s[0] * values[0] + s[1] * values[1] + s[2] * values[2];
                T v2 = s[3] * values[0] + s[4] * values[1] + s[5] * values[2];
                T v3 = s[6] * values[0] + s[7] * values[1] + s[8] * values[2];
                out_values[0] = v1;
                out_values[1] = v2;
                out_values[2] = v3;
            }
        }

/*
 TODO: !!!!!!

    void colorSaturation(double saturation) noexcept;
    bool whiteBalance(const GrRGB& color) noexcept;
    bool buildCIEXYZColorMatrix(Color::Space colorSpace, GrColorCalibrationReference calibrationReference, const GrRGB& r, const GrRGB& g, const GrRGB& b, const GrRGB& w) noexcept;
    bool buildCIEXYZColorMatrix(const GrVec2& rv, const GrVec2& gv, const GrVec2& bv, const GrVec2& wv) noexcept;
    void multiplyWithColor(const GrRGB& color) noexcept;
    bool divideByColor(const GrRGB& color) noexcept;
*/

    public:
        T m_data[3][3];
    };


    // Standard types.
    using Mat3f = Mat3<float>;      ///< 32 bit floating point
    using Mat3d = Mat3<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainMat3_hpp
