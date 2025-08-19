//
//  Mat4.hpp
//
//  Created by Roald Christesen on 04.10.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainMat4_hpp
#define GrainMat4_hpp

#include "Grain.hpp"
#include "Vec3.hpp"


namespace Grain {

    /**
     *  @brief 4x4 Matrix.
     *
     *  `Mat4` represents 4x4 matrices, a foundational tool for a wide array of applications involving
     *  3D transformations and computations.
     *  This templated class offers versatility in numeric representation by supporting various data types.
     *  Predefined specialized versions include datatypes float and double, referred to as `Mat4f`
     *  and `Mat4d` respectively.
     *
     *  `Mat4` plays an integral role in numerous tasks such as 3D graphics transformations, camera
     *  transformations, and homogeneous coordinate transformations. It is vital in computer graphics,
     *  computer vision, game development, robotics, and simulations.
     *
     *  `Mat4` empowers you to handle complex spatial transformations with precision and efficiency,
     *  facilitating accurate computations in a 3D environment.
     */
    template <class T>
    class Mat4 {
    public:
        Mat4() noexcept { identity(); }
        Mat4(T v00, T v01, T v02, T v03, T v10, T v11, T v12, T v13, T v20, T v21, T v22, T v23, T v30, T v31, T v32, T v33) noexcept {
            T* d = mutDataPtr();
            d[0]  = v00; d[1]  = v10; d[2]  = v20; d[3]  = v30;
            d[4]  = v01; d[5]  = v11; d[6]  = v21; d[7]  = v31;
            d[8]  = v02; d[9]  = v12; d[10] = v22; d[11] = v32;
            d[12] = v03; d[13] = v13; d[14] = v23; d[15] = v33;
        }
        Mat4(T* v, bool row_order = true) noexcept { set(v, row_order); }
        Mat4(const char* csv) noexcept {
            // TODO: !!!!!!!! Implement
        }

        [[nodiscard]] virtual const char* className() const noexcept { return "Mat4"; }


        friend std::ostream& operator << (std::ostream& os, const Mat4* o) {
            o == nullptr ? os << "Mat4 nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Mat4& o) {
            auto d = o.dataPtr();
            os << d[0] << ", " << d[1] << ", " << d[2] << ", " << d[3] << " | ";
            os << d[4] << ", " << d[5] << ", " << d[6] << ", " << d[7] << " | ";
            os << d[8] << ", " << d[9] << ", " << d[10] << ", " << d[11] << " | ";
            os << d[12] << ", " << d[13] << ", " << d[14] << ", " << d[15];
            return os;
        }

        Mat4& operator *= (const Mat4& other) { other.mul(*this, *this); return *this; }
        Mat4 operator * (const Mat4& other) { Mat4 result; other.mul(*this, result); return result; }


        T* mutDataPtr() noexcept { return &m_data[0][0]; }
        const T* dataPtr() const noexcept { return &m_data[0][0]; }

        T valueAtRowColumn(int32_t row, int32_t column) const noexcept {
            if (row >= 0 && row < 4 && column >= 0 && column < 4) {
                return m_data[row][column];
            }
            else {
                return 0;
            }
        }

        void set(T v00, T v01, T v02, T v03, T v10, T v11, T v12, T v13, T v20, T v21, T v22, T v23, T v30, T v31, T v32, T v33) noexcept {
            T* d = mutDataPtr();
            d[0]  = v00; d[1]  = v10; d[2]  = v20; d[3]  = v30;
            d[4]  = v01; d[5]  = v11; d[6]  = v21; d[7]  = v31;
            d[8]  = v02; d[9]  = v12; d[10] = v22; d[11] = v32;
            d[12] = v03; d[13] = v13; d[14] = v23; d[15] = v33;
        }

        void set(const T* v, bool row_order = true) noexcept {
            if (v) {
                T* d = mutDataPtr();
                if (row_order) {
                    for (int i = 0; i < 16; i++)
                        *d++ = *v++;
                }
                else {
                    d[0] = v[0];  d[4] = v[1];  d[8]  = v[2];  d[12] = v[3];
                    d[1] = v[4];  d[5] = v[5];  d[9]  = v[6];  d[13] = v[7];
                    d[2] = v[8];  d[6] = v[9];  d[10] = v[10]; d[14] = v[11];
                    d[3] = v[12]; d[7] = v[13]; d[11] = v[14]; d[15] = v[15];
                }
            }
        }

        void clear() noexcept {
            T* d = mutDataPtr();
            for (int i = 0; i < 16; i++)
                *d++ = 0;
        }

        void identity() noexcept {
            T* d = mutDataPtr();
            d[0]  = 1; d[1]  = 0; d[2]  = 0; d[3]  = 0;
            d[4]  = 0; d[5]  = 1; d[6]  = 0; d[7]  = 0;
            d[8]  = 0; d[9]  = 0; d[10] = 1; d[11] = 0;
            d[12] = 0; d[13] = 0; d[14] = 0; d[15] = 1;
        }


        void translateX(T t) noexcept { translate(t, 0, 0); }
        void translateY(T t) noexcept { translate(0, t, 0); }
        void translateZ(T t) noexcept { translate(0, 0, t); }

        void translate(T tx, T ty, T tz) noexcept {
            Mat4<T> m(1, 0, 0, tx, 0, 1, 0, ty, 0, 0, 1, tz, 0, 0, 0, 1);
            mul(m);
        }

        void translate(const Vec3<T>& v) noexcept {
            Mat4<T> m(1, 0, 0, v.m_x, 0, 1, 0, v.m_y, 0, 0, 1, v.m_z, 0, 0, 0, 1);
            mul(m);
        }

        void scale(T s) noexcept { scale(s, s, s); }
        void scaleX(T s) noexcept { scale(s, 1, 1); }
        void scaleY(T s) noexcept { scale(1, s, 1); }
        void scaleZ(T s) noexcept { scale(1, 1, s); }

        void scale(T sx, T sy, T sz) noexcept {
            Mat4<T> m(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1);
            mul(m);
        }

        void rotateX(T angle) noexcept {
            T rad = angle * std::numbers::pi / 180.0;
            T c = std::cos(rad);
            T s = std::sin(rad);
            Mat4<T> m(1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
            mul(m);
        }

        void rotateY(T angle) noexcept {
            T rad = angle * std::numbers::pi / 180.0;
            T c = std::cos(rad);
            T s = std::sin(rad);
            Mat4<T> m(c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1);
            mul(m);
        }

        void rotateZ(T angle) noexcept {
            T rad = angle * std::numbers::pi / 180.0;
            T c = std::cos(rad);
            T s = std::sin(rad);
            Mat4<T> m(c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            mul(m);
        }

        void rotate(T angle, const Vec3<T>& axis) noexcept {
            rotate(angle, axis.m_x, axis.m_y, axis.m_z);
        }

        void rotate(T angle, T x, T y, T z) noexcept {

            T rad = angle * std::numbers::pi / 180.0;
            T c = std::cos(rad);
            T cc = 1.0 - c;
            T s = std::sin(rad);
            T len = std::sqrt(x * x + y * y + z * z);
            x /= len;
            y /= len;
            z /= len;

            Mat4<T> m;

            m.m_data[0][0] = x * x + (1.0 - x * x) * c;
            m.m_data[0][1] = x * y * cc + z * s;
            m.m_data[0][2] = x * z * cc - y * s;
            m.m_data[0][3] = 0.0;

            m.m_data[1][0] = x * y * cc - z * s;
            m.m_data[1][1] = y * y + (1.0 - y * y) * c;
            m.m_data[1][2] = y * z * cc + x * s;
            m.m_data[1][3] = 0.0;

            m.m_data[2][0] = x * z * cc + y * s;
            m.m_data[2][1] = y * z * cc - x * s;
            m.m_data[2][2] = z * z + (1.0 - z * z) * c;
            m.m_data[2][3] = 0.0;

            m.m_data[3][0] = 0.0;
            m.m_data[3][1] = 0.0;
            m.m_data[3][2] = 0.0;
            m.m_data[3][3] = 1.0;

            mul(m);
        }


        T determinant() noexcept {
            // TODO: Check!
            T* m = mutDataPtr();
            return
                    m[0]  * coFactor(m[5], m[9], m[13], m[6], m[10], m[14], m[7], m[11], m[15]) -
                    m[4]  * coFactor(m[1], m[9], m[13], m[2], m[10], m[14], m[3], m[11], m[15]) +
                    m[8]  * coFactor(m[1], m[5], m[13], m[2], m[6],  m[14], m[3], m[7],  m[15]) -
                    m[12] * coFactor(m[1], m[5], m[9],  m[2], m[6],  m[10], m[3], m[7],  m[11]);
        }

        void transpose() noexcept {
            // TODO: Check!
            T* m = mutDataPtr();
            T temp;
            temp = m[1];  m[1]  = m[4];  m[4]  = temp;
            temp = m[2];  m[2]  = m[8];  m[8]  = temp;
            temp = m[3];  m[3]  = m[12]; m[12] = temp;
            temp = m[6];  m[6]  = m[9];  m[9]  = temp;
            temp = m[7];  m[7]  = m[13]; m[13] = temp;
            temp = m[11]; m[11] = m[14]; m[14] = temp;
        }

        bool invert() noexcept { return invert(*this); }

        bool invert(Mat4<T>& out_matrix) const noexcept {

            // TODO: Check!

            const T* m = dataPtr();
            T* d = out_matrix.mutDataPtr();

            // Get cofactors of minor matrices
            T cf0 = coFactor(m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]);
            T cf1 = coFactor(m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]);
            T cf2 = coFactor(m[4], m[5], m[7], m[8], m[9],  m[11], m[12], m[13], m[15]);
            T cf3 = coFactor(m[4], m[5], m[6], m[8], m[9],  m[10], m[12], m[13], m[14]);

            // Get determinant
            T det = m[0] * cf0 - m[1] * cf1 + m[2] * cf2 - m[3] * cf3;
            if (std::fabs(det) < std::numeric_limits<double>::epsilon()) {
                out_matrix.identity();
                return false;
            }

            // Get rest of cofactors for adj(M)
            T cf4  = coFactor(m[1], m[2], m[3], m[9], m[10], m[11], m[13], m[14], m[15]);
            T cf5  = coFactor(m[0], m[2], m[3], m[8], m[10], m[11], m[12], m[14], m[15]);
            T cf6  = coFactor(m[0], m[1], m[3], m[8], m[9],  m[11], m[12], m[13], m[15]);
            T cf7  = coFactor(m[0], m[1], m[2], m[8], m[9],  m[10], m[12], m[13], m[14]);
            T cf8  = coFactor(m[1], m[2], m[3], m[5], m[6],  m[7],  m[13], m[14], m[15]);
            T cf9  = coFactor(m[0], m[2], m[3], m[4], m[6],  m[7],  m[12], m[14], m[15]);
            T cf10 = coFactor(m[0], m[1], m[3], m[4], m[5],  m[7],  m[12], m[13], m[15]);
            T cf11 = coFactor(m[0], m[1], m[2], m[4], m[5],  m[6],  m[12], m[13], m[14]);
            T cf12 = coFactor(m[1], m[2], m[3], m[5], m[6],  m[7],  m[9],  m[10], m[11]);
            T cf13 = coFactor(m[0], m[2], m[3], m[4], m[6],  m[7],  m[8],  m[10], m[11]);
            T cf14 = coFactor(m[0], m[1], m[3], m[4], m[5],  m[7],  m[8],  m[9],  m[11]);
            T cf15 = coFactor(m[0], m[1], m[2], m[4], m[5],  m[6],  m[8],  m[9],  m[10]);

            // Build inverse matrix = adj(M) / det(M)
            // Adjugate of M is the transpose of the cofactor matrix of M
            T inv_det = 1.0 / det;
            d[0]  =  inv_det * cf0;
            d[1]  = -inv_det * cf4;
            d[2]  =  inv_det * cf8;
            d[3]  = -inv_det * cf12;
            d[4]  = -inv_det * cf1;
            d[5]  =  inv_det * cf5;
            d[6]  = -inv_det * cf9;
            d[7]  =  inv_det * cf13;
            d[8]  =  inv_det * cf2;
            d[9]  = -inv_det * cf6;
            d[10] =  inv_det * cf10;
            d[11] = -inv_det * cf14;
            d[12] = -inv_det * cf3;
            d[13] =  inv_det * cf7;
            d[14] = -inv_det * cf11;
            d[15] =  inv_det * cf15;

            return true;
        }


        void mul(const Mat4<T>& m) noexcept {

            const T* a = dataPtr();
            const T* b = m.dataPtr();
            T temp[16];

            temp[0]  = a[0]  * b[0] + a[1]  * b[4] + a[2]  * b[8]  + a[3]  * b[12];
            temp[1]  = a[0]  * b[1] + a[1]  * b[5] + a[2]  * b[9]  + a[3]  * b[13];
            temp[2]  = a[0]  * b[2] + a[1]  * b[6] + a[2]  * b[10] + a[3]  * b[14];
            temp[3]  = a[0]  * b[3] + a[1]  * b[7] + a[2]  * b[11] + a[3]  * b[15];
            temp[4]  = a[4]  * b[0] + a[5]  * b[4] + a[6]  * b[8]  + a[7]  * b[12];
            temp[5]  = a[4]  * b[1] + a[5]  * b[5] + a[6]  * b[9]  + a[7]  * b[13];
            temp[6]  = a[4]  * b[2] + a[5]  * b[6] + a[6]  * b[10] + a[7]  * b[14];
            temp[7]  = a[4]  * b[3] + a[5]  * b[7] + a[6]  * b[11] + a[7]  * b[15];
            temp[8]  = a[8]  * b[0] + a[9]  * b[4] + a[10] * b[8]  + a[11] * b[12];
            temp[9]  = a[8]  * b[1] + a[9]  * b[5] + a[10] * b[9]  + a[11] * b[13];
            temp[10] = a[8]  * b[2] + a[9]  * b[6] + a[10] * b[10] + a[11] * b[14];
            temp[11] = a[8]  * b[3] + a[9]  * b[7] + a[10] * b[11] + a[11] * b[15];
            temp[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8]  + a[15] * b[12];
            temp[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9]  + a[15] * b[13];
            temp[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
            temp[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

            memcpy(mutDataPtr(), temp, sizeof(T) * 16);
        }

        void mulSwapped(const Mat4<T>& m) noexcept {

            const T* a = m.dataPtr();
            const T* b = dataPtr();
            T temp[16];

            temp[0]  = a[0]  * b[0] + a[1]  * b[4] + a[2]  * b[8]  + a[3]  * b[12];
            temp[1]  = a[0]  * b[1] + a[1]  * b[5] + a[2]  * b[9]  + a[3]  * b[13];
            temp[2]  = a[0]  * b[2] + a[1]  * b[6] + a[2]  * b[10] + a[3]  * b[14];
            temp[3]  = a[0]  * b[3] + a[1]  * b[7] + a[2]  * b[11] + a[3]  * b[15];
            temp[4]  = a[4]  * b[0] + a[5]  * b[4] + a[6]  * b[8]  + a[7]  * b[12];
            temp[5]  = a[4]  * b[1] + a[5]  * b[5] + a[6]  * b[9]  + a[7]  * b[13];
            temp[6]  = a[4]  * b[2] + a[5]  * b[6] + a[6]  * b[10] + a[7]  * b[14];
            temp[7]  = a[4]  * b[3] + a[5]  * b[7] + a[6]  * b[11] + a[7]  * b[15];
            temp[8]  = a[8]  * b[0] + a[9]  * b[4] + a[10] * b[8]  + a[11] * b[12];
            temp[9]  = a[8]  * b[1] + a[9]  * b[5] + a[10] * b[9]  + a[11] * b[13];
            temp[10] = a[8]  * b[2] + a[9]  * b[6] + a[10] * b[10] + a[11] * b[14];
            temp[11] = a[8]  * b[3] + a[9]  * b[7] + a[10] * b[11] + a[11] * b[15];
            temp[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8]  + a[15] * b[12];
            temp[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9]  + a[15] * b[13];
            temp[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
            temp[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

            memcpy(mutDataPtr(), temp, sizeof(T) * 16);
        }

        void mul(const Mat4<T>& m, Mat4<T>& outMatrix) const noexcept {

            const T* a = dataPtr();
            const T* b = m.dataPtr();
            T temp[16];

            temp[0]  = a[0]  * b[0] + a[1]  * b[4] + a[2]  * b[8]  + a[3]  * b[12];
            temp[1]  = a[0]  * b[1] + a[1]  * b[5] + a[2]  * b[9]  + a[3]  * b[13];
            temp[2]  = a[0]  * b[2] + a[1]  * b[6] + a[2]  * b[10] + a[3]  * b[14];
            temp[3]  = a[0]  * b[3] + a[1]  * b[7] + a[2]  * b[11] + a[3]  * b[15];
            temp[4]  = a[4]  * b[0] + a[5]  * b[4] + a[6]  * b[8]  + a[7]  * b[12];
            temp[5]  = a[4]  * b[1] + a[5]  * b[5] + a[6]  * b[9]  + a[7]  * b[13];
            temp[6]  = a[4]  * b[2] + a[5]  * b[6] + a[6]  * b[10] + a[7]  * b[14];
            temp[7]  = a[4]  * b[3] + a[5]  * b[7] + a[6]  * b[11] + a[7]  * b[15];
            temp[8]  = a[8]  * b[0] + a[9]  * b[4] + a[10] * b[8]  + a[11] * b[12];
            temp[9]  = a[8]  * b[1] + a[9]  * b[5] + a[10] * b[9]  + a[11] * b[13];
            temp[10] = a[8]  * b[2] + a[9]  * b[6] + a[10] * b[10] + a[11] * b[14];
            temp[11] = a[8]  * b[3] + a[9]  * b[7] + a[10] * b[11] + a[11] * b[15];
            temp[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8]  + a[15] * b[12];
            temp[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9]  + a[15] * b[13];
            temp[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
            temp[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

            memcpy(outMatrix.mutDataPtr(), temp, sizeof(T) * 16);
        }


        void transformVec3(Vec3<T>& v) const noexcept {
            T x = v.m_x * m_data[0][0] + v.m_y * m_data[1][0] + v.m_z * m_data[2][0] + m_data[3][0];
            T y = v.m_x * m_data[0][1] + v.m_y * m_data[1][1] + v.m_z * m_data[2][1] + m_data[3][1];
            T z = v.m_x * m_data[0][2] + v.m_y * m_data[1][2] + v.m_z * m_data[2][2] + m_data[3][2];
            T w = v.m_x * m_data[0][3] + v.m_y * m_data[1][3] + v.m_z * m_data[2][3] + m_data[3][3];
            if (w != 0.0) {
                v.m_x = x / w;
                v.m_y = y / w;
                v.m_z = z / w;
            }
            else {
                v.m_x = x;
                v.m_y = y;
                v.m_z = z;
            }
        }

        void transform3Array(T* data, int32_t n) const noexcept {
            if (data && n > 0) {
                T* d = data;
                for (int32_t i = 0; i < n; i++) {
                    T x = m_data[0][0] * d[0] + m_data[1][0] * d[1] + m_data[2][0] * d[2] + m_data[3][0];
                    T y = m_data[0][1] * d[0] + m_data[1][1] * d[1] + m_data[2][1] * d[2] + m_data[3][1];
                    T z = m_data[0][2] * d[0] + m_data[1][2] * d[1] + m_data[2][2] * d[2] + m_data[3][2];
                    T w = m_data[0][3] * d[0] + m_data[1][3] * d[1] + m_data[2][3] * d[2] + m_data[3][3];
                    if (w != 0.0) {
                        d[0] = x / w;
                        d[1] = y / w;
                        d[2] = z / w;
                    }
                    else {
                        d[0] = x;
                        d[1] = y;
                        d[2] = z;
                    }
                    d += 3;
                }
            }
        }

        void transformVec3Dir(Vec3<T>& dir) const noexcept {
            T x = m_data[0][0] * dir.m_x + m_data[1][0] * dir.m_y + m_data[2][0] * dir.m_z;
            T y = m_data[0][1] * dir.m_x + m_data[1][1] * dir.m_y + m_data[2][1] * dir.m_z;
            T z = m_data[0][2] * dir.m_x + m_data[1][2] * dir.m_y + m_data[2][2] * dir.m_z;
            T w = m_data[0][3] * dir.m_x + m_data[1][3] * dir.m_y + m_data[2][3] * dir.m_z;
            if (w != 0.0) {
                dir.m_x = x / w;
                dir.m_y = y / w;
                dir.m_z = z / w;
            }
            else {
                dir.m_x = x;
                dir.m_y = y;
                dir.m_z = z;
            }
        }

        void transform3DirArray(T* data, int32_t n) const noexcept {
            if (data && n > 0) {
                T* d = data;
                for (int32_t i = 0; i < n; i++) {
                    T x = m_data[0][0] * d[0] + m_data[1][0] * d[1] + m_data[2][0] * d[2];
                    T y = m_data[0][1] * d[0] + m_data[1][1] * d[1] + m_data[2][1] * d[2];
                    T z = m_data[0][2] * d[0] + m_data[1][2] * d[1] + m_data[2][2] * d[2];
                    T w = m_data[0][3] * d[0] + m_data[1][3] * d[1] + m_data[2][3] * d[2];
                    if (w != 0.0) {
                        d[0] = x / w;
                        d[1] = y / w;
                        d[2] = z / w;
                    }
                    else {
                        d[0] = x;
                        d[1] = y;
                        d[2] = z;
                    }
                    d += 3;
                }
            }
        }


        // 3D projections.

        void lookAtLeftHand(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up) noexcept {

            Vec3<T> z = target - eye; z.normalize();
            Vec3<T> x = up.cross(z); x.normalize();
            Vec3<T> y = z.cross(x);
            Vec3<T> t(-x.dot(eye), -y.dot(eye), -z.dot(eye));

            set(x.m_x, x.m_y, x.m_z, t.m_x, y.m_x, y.m_y, y.m_z, t.m_y, z.m_x, z.m_y, z.m_z, t.m_z, 0, 0, 0, 1);
        }

        void orthoLeftHand(T left, T right, T bottom, T top, T near, T far) noexcept {

            set(2.0 / (right - left), 0, 0, (left + right) / (left - right),
                0, 2.0 / (top - bottom), 0, (top + bottom) / (bottom - top),
                0, 0, 1.0 / (far - near), near / (near - far),
                0, 0, 0, 1);
        }


        void perspectiveLeftHand(T view_angle, T aspect, T near, T far) noexcept {

            double x_scale = 1.0 / std::tan(static_cast<double>(view_angle) / 180.0 * std::numbers::pi * 0.5);
            double y_scale = x_scale / aspect;
            double z_scale = far / (far - near);

            m_data[0][0] = x_scale;
            m_data[0][1] = 0;
            m_data[0][2] = 0;
            m_data[0][3] = 0;

            m_data[1][0] = 0;
            m_data[1][1] = y_scale;
            m_data[1][2] = 0;
            m_data[1][3] = 0;

            m_data[2][0] = 0;
            m_data[2][1] = 0;
            m_data[2][2] = z_scale;
            m_data[2][3] = 1;

            m_data[3][0] = 0;
            m_data[3][1] = 0;
            m_data[3][2] = -near * z_scale;
            m_data[3][3] = 0;
        }


        void trackball(double dx, double dy) noexcept {

            if (dx < -1.0) { dx = -1.0; } else if (dx > 1.0) { dx = 1.0; }
            if (dy < -1.0) { dy = -1.0; } else if (dy > 1.0) { dy = 1.0; }

            // Calculate the angle of rotation (in radians) based on dx and dy
            double angle = std::sqrt(dx * dx + dy * dy) * std::numbers::pi;

            // Calculate the axis of rotation
            Vec3d axis(dy, dx, 0);
            double view_space_magnitude = std::sqrt(axis.m_x * axis.m_x + axis.m_y * axis.m_y + axis.m_z * axis.m_z);
            if (Safe::canSafelyDivideBy(view_space_magnitude)) {
                axis *= 1.0 / view_space_magnitude;     // Normalize the axis
            }

            // Calculate the trackball transformation matrix
            double c = std::cos(angle);
            double s = std::sin(angle);
            double t = 1.0 - c;
            double x = axis.m_x;
            double y = axis.m_y;
            double z = axis.m_z;

            set(
                    t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0.0,
                    t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0.0,
                    t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0.0,
                    0.0, 0.0, 0.0, 1.0);
        }


        inline T coFactor(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8) const noexcept {
            return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
        }

    public:
        T m_data[4][4];
    };


    // Standard types.
    using Mat4f = Mat4<float>;      ///< 32 bit floating point
    using Mat4d = Mat4<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainMat4_hpp
