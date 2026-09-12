/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <algorithm>
#include <iostream>
#include <cassert>


namespace NCL::Maths {

    template <typename T, uint32_t n>
    struct VectorTemplate {
        T array[n];

        inline T operator[](int i) const {
            return ((T*)this)[i];
        }

        inline T& operator[](int i) {
            return ((T*)this)[i];
        }
    };

    using Vector2 = VectorTemplate<float, 2>;
    using Vector3 = VectorTemplate<float, 3>;
    using Vector4 = VectorTemplate<float, 4>;

    using Vector2d = VectorTemplate<double, 2>;
    using Vector3d = VectorTemplate<double, 3>;
    using Vector4d = VectorTemplate<double, 4>;

    using Vector2i = VectorTemplate<int32_t, 2>;
    using Vector3i = VectorTemplate<int32_t, 3>;
    using Vector4i = VectorTemplate<int32_t, 4>;

    using Vector2ui = VectorTemplate<uint32_t, 2>;
    using Vector3ui = VectorTemplate<uint32_t, 3>;
    using Vector4ui = VectorTemplate<uint32_t, 4>;


    template <typename T>
    struct VectorTemplate<T, 2> {
        union {
            T array[2];
            struct {
                T x;
                T y;
            };
        };

        VectorTemplate<T, 2>() : x(0), y(0) {
        }

        VectorTemplate<T, 2>(T inX, T inY) : x(inX), y(inY) {
        }

        VectorTemplate<T, 2>(VectorTemplate<T, 3> v) : x(v[0]), y(v[1]) {
        }


        T operator[](int i) const {
            return ((T*)this)[i];
        }
        T& operator[](int i) {
            return ((T*)this)[i];
        }
    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const VectorTemplate<T, 3>& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }

    template <typename T>
    struct VectorTemplate<T, 3> {
        union {
            T array[3];
            struct {
                T x;
                T y;
                T z;
            };
        };

        VectorTemplate<T, 3>() : x(0), y(0), z(0) {
        }

        VectorTemplate<T, 3>(T inX, T inY, T inZ) : x(inX), y(inY), z(inZ) {
        }

        VectorTemplate<T, 3>(VectorTemplate<T, 2> v, T inZ) : x(v.array[0]), y(v.array[1]), z(inZ) {
        }

        VectorTemplate<T, 3>(VectorTemplate<T, 4> v) : x(v[0]), y(v[1]), z(v[2]) {
        }

        // 转换为 const 指针
        operator const T* () const {
            return &x;  // 假设 x, y, z 连续存储
        }

        // 转换为非 const 指针
        operator T* () {
            return &x;  // 假设 x, y, z 连续存储
        }

        T Dot(const VectorTemplate<T, 3>& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        bool operator==(const VectorTemplate<T, 3>& other) const {
            return (x == other.x) && (y == other.y) && (z == other.z);
        }

        bool operator!=(const VectorTemplate<T, 3>& other) const {
            return !(*this == other);
        }

        float LengthSquared() const {
            return x * x + y * y + z * z;
        }

        T operator[](int i) const {
            assert(i >= 0 && i < 3);  // 添加范围检查以避免越界访问
            return (&x)[i];
        }
        T& operator[](int i) {
            assert(i >= 0 && i < 3);  // 添加范围检查以避免越界访问
            return (&x)[i];
        }

        VectorTemplate<T, 3> operator+(const T& scalar) const {
            return { x + scalar, y + scalar, z + scalar };
        }
        VectorTemplate<T, 3> operator-(const T& scalar) const {
            return { x - scalar, y - scalar, z - scalar };
        }

        VectorTemplate<T, 3> operator-() const {
            return { -x, -y, -z };
        }
        VectorTemplate<T, 3> operator+() const {
            return *this;  // 返回自身
        }

        // **标量乘法（重载 `*` 和 `/`）**
        VectorTemplate<T, 3> operator*(T scalar) const {
            return VectorTemplate<T, 3>(x * scalar, y * scalar, z * scalar);
        }
        VectorTemplate<T, 3>& operator*=(T scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }
        VectorTemplate<T, 3> operator/(T scalar) const {
            return VectorTemplate<T, 3>(x / scalar, y / scalar, z / scalar);
        }
        VectorTemplate<T, 3>& operator/=(T scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        friend VectorTemplate<T, 3> operator*(T scalar, const VectorTemplate<T, 3>& vec) {
            return VectorTemplate<T, 3>(scalar * vec.x, scalar * vec.y, scalar * vec.z);
        }

    };

    template <typename T>
    struct VectorTemplate<T, 4> {
        union {
            T array[4];
            struct {
                T x;
                T y;
                T z;
                T w;
            };
        };

        VectorTemplate<T, 4>() : x(0), y(0), z(0), w(0) {
        }

        VectorTemplate<T, 4>(T inX, T inY, T inZ, T inW) : x(inX), y(inY), z(inZ), w(inW) {
        }

        VectorTemplate<T, 4>(VectorTemplate<T, 2> v, T inZ, T inW) : x(v.array[0]), y(v.array[1]), z(inZ), w(inW) {
        }

        VectorTemplate<T, 4>(VectorTemplate<T, 3> v, T inW) : x(v.array[0]), y(v.array[1]), z(v.array[2]), w(inW) {
        }

        friend std::ostream& operator<<(std::ostream& os, const VectorTemplate<T, 4>& vec) {
            os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
            return os;
        }

        T operator[](int i) const {
            return ((T*)this)[i];
        }
        T& operator[](int i) {
            return ((T*)this)[i];
        }
    };


    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator+(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] + b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator-(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] - b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator-(const VectorTemplate<T, n>& a) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = -a[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator*(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] * b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator/(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] / b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator*(const VectorTemplate<T, n>& a, const T& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] * b;
        }
        return answer;
    }

    template <typename T, uint32_t n>
    constexpr VectorTemplate<T, n>  operator/(const VectorTemplate<T, n>& a, const T& b) {
        VectorTemplate<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] / b;
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator+=(VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] + b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator-=(VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] - b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator*=(VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] * b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator/=(VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] / b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator*=(VectorTemplate<T, n>& a, const T& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] * b;
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline VectorTemplate<T, n>& operator/=(VectorTemplate<T, n>& a, const T& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] / b;
        }
        return a;
    }



    namespace Vector {
        template <typename T>
        VectorTemplate<T, 3> Cross(const VectorTemplate<T, 3>& a, const VectorTemplate<T, 3>& b) {
            VectorTemplate<T, 3> result;
            result.x = (a.y * b.z) - (a.z * b.y);
            result.y = (a.z * b.x) - (a.x * b.z);
            result.z = (a.x * b.y) - (a.y * b.x);
            return result;
        }

        template <typename T, uint32_t n>
        T Dot(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
            T result(0);
            for (int i = 0; i < n; ++i) {
                result += a[i] * b[i];
            }
            return result;
        }

        template <typename T, uint32_t n>
        constexpr T LengthSquared(const VectorTemplate<T, n>& a) {
            T result(0);
            for (int i = 0; i < n; ++i) {
                result += a[i] * a[i];
            }
            return result;
        }

        template <typename T, uint32_t n>
        T Length(const VectorTemplate<T, n>& a) {
            return std::sqrt(LengthSquared(a));
        }

        template <typename T, uint32_t n>
        constexpr VectorTemplate<T, n> Normalise(const VectorTemplate<T, n>& a) {
            VectorTemplate<T, n> result;
            T l = Vector::Length(a);
            if (l > 0.0f) {
                T r = T(1.0) / l;
                for (int i = 0; i < n; ++i) {
                    result.array[i] = a.array[i] * r;
                }
            }
            return result;
        }

        template <typename T, uint32_t n>
        constexpr T		GetMinElement(const VectorTemplate<T, n>& a) {
            float v = a.array[0];
            for (int i = 1; i < n; ++i) {
                v = std::min(v, a.array[i]);
            }
            return v;
        }

        template <typename T, uint32_t n>
        constexpr T		GetMaxElement(const VectorTemplate<T, n>& a) {
            float v = a.array[0];
            for (int i = 1; i < n; ++i) {
                v = std::max(v, a.array[i]);
            }
            return v;
        }

        template <typename T, uint32_t n>
        constexpr T		GetAbsMaxElement(const VectorTemplate<T, n>& a) {
            float v = std::abs(a.array[0]);
            for (int i = 1; i < n; ++i) {
                v = std::max(v, std::abs(a.array[i]));
            }
            return v;
        }

        template <typename T, uint32_t n>
        constexpr VectorTemplate<T, n>        Clamp(const VectorTemplate<T, n>& input, const VectorTemplate<T, n>& mins, const VectorTemplate<T, n>& maxs) {
            VectorTemplate<T, n> output;
            for (int i = 0; i < n; ++i) {
                output.array[i] = std::clamp(input.array[i], mins.array[i], maxs.array[i]);
            }
            return output;
        }

        static Vector3 Orthogonal(const Vector3& v) {
            if (fabs(v.x) < fabs(v.y) && fabs(v.x) < fabs(v.z)) {
                return Vector3(1, 0, 0);
            }
            else if (fabs(v.y) < fabs(v.z)) {
                return Vector3(0, 1, 0);
            }
            else {
                return Vector3(0, 0, 1);
            }
        }

        template <typename T, uint32_t n>
        static T Angle(const VectorTemplate<T, n>& a, const VectorTemplate<T, n>& b) {
            T dotProduct = Dot(a, b);
            T lenA = Length(a);
            T lenB = Length(b);

            if (lenA < 1e-6f || lenB < 1e-6f) {
                return T(0); // 处理零向量
            }

            T cosTheta = dotProduct / (lenA * lenB);
            cosTheta = std::clamp(cosTheta, T(-1), T(1)); // 防止浮点误差
            return std::acos(cosTheta) * (180.0f / 3.14159265358979323846f); // 弧度转度数
        }

    }

}