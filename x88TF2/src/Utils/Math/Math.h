#pragma once

#include "../Vector/Vector.h"

#include <cmath>
#include <cfloat>
#include <algorithm>
#include <array>

#undef min
#undef max

#define PI 3.14159265358979323846
#define M_RADPI 57.295779513082
#define DEG2RAD(x) ((float)(x) * (float)((float)(PI) / 180.0f))
#define RAD2DEG(x) ((float)(x) * (float)(180.0f / (float)(PI)))

class Vector4D
{
public:
	float x, y, z, w;
};

using matrix3x4_t = float[3][4];
using Vector = Vec3;
using Vector2D = Vec2;
using QAngle = Vec3;
using RadianEuler = Vec3;
using Quaternion = Vector4D;

class VMatrix
{
public:
	Vector m[4][4];

public:
	inline const matrix3x4_t& As3x4() const {
		return *((const matrix3x4_t*)this);
	}
};

#pragma warning (push)
#pragma warning (disable : 26451)
#pragma warning (disable : 4244)

#define floatCompare(x, y) (fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

namespace Math
{
    // ========== AMALGAM STYLE UTILS ==========

    inline double FastSqrt(double n) {
        return std::sqrt(n);
    }

    inline void SinCos(float radians, float* sine, float* cosine)
    {
        *sine = sinf(radians);
        *cosine = cosf(radians);
    }

    // Normaliza um único ângulo para o range [-180, 180]
    inline float NormalizeAngle(float ang)
    {
        // CORREÇÃO: Removido std::isfinite para compatibilidade
        return (!isfinite(ang)) ? 0.0f : std::remainder(ang, 360.0f);
    }

    inline void NormalizeAngles(Vec3& v)
    {
        v.x = NormalizeAngle(v.x);
        v.y = NormalizeAngle(v.y);
        v.z = 0.0f;
    }

    inline void ClampAngles(Vec3& v)
    {
        // CORREÇÃO: Substituindo std::clamp por lógica manual
        v.x = (NormalizeAngle(v.x) < -89.0f) ? -89.0f : (NormalizeAngle(v.x) > 89.0f) ? 89.0f : NormalizeAngle(v.x);
        v.y = NormalizeAngle(v.y);
        v.z = 0.0f;
    }

    inline float AngleDiff(float destAngle, float srcAngle)
    {
        float delta = NormalizeAngle(destAngle - srcAngle);
        return delta;
    }

    inline float Approach(float target, float value, float speed)
    {
        float delta = target - value;

        if (fabsf(delta) > speed)
        {
            value += speed * (delta > 0.0f ? 1.0f : -1.0f);
        }
        else
        {
            value = target;
        }

        return value;
    }

    inline Vec3 Approach(const Vec3& target, const Vec3& value, float speed)
    {
        Vec3 diff = target - value;
        float len = diff.Length();

        if (len > speed)
        {
            return value + diff.Normalized() * speed;
        }
        return target;
    }

    inline Vec3 AngleLerp(const Vec3& from, const Vec3& to, float t)
    {
        Vec3 delta = to - from;
        NormalizeAngles(delta);
        return from + delta * t;
    }

    inline auto GetRotatedPosition(Vec3 start, const float rotation, const float distance)
    {
        const auto rad = DEG2RAD(rotation);
        start.x += cosf(rad) * distance;
        start.y += sinf(rad) * distance;

        return start;
    }

    inline bool SolveQuadratic(float a, float b, float c, float& x1, float& x2)
    {
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f)
            return false;

        if (a == 0.0f)
        {
            if (b == 0.0f)
                return false;

            x1 = x2 = -c / b;
            return true;
        }

        float sqrtDisc = sqrtf(discriminant);
        x1 = (-b + sqrtDisc) / (2.0f * a);
        x2 = (-b - sqrtDisc) / (2.0f * a);

        return true;
    }

    inline Vec2 RotatePoint(const Vec2& point, const Vec2& center, float angle)
    {
        float rad = DEG2RAD(angle);
        float s = sinf(rad);
        float c = cosf(rad);

        Vec2 p = point - center;
        Vec2 rotated;
        rotated.x = p.x * c - p.y * s;
        rotated.y = p.x * s + p.y * c;

        return rotated + center;
    }

    inline Vec3 Normalized2D(const Vec3& v)
    {
        float len = sqrtf(v.x * v.x + v.y * v.y);

        if (len <= 0.0f)
            return Vec3(0.0f, 0.0f, v.z);

        return Vec3(v.x / len, v.y / len, v.z);
    }

    inline void VectorAngles(const Vec3& forward, Vec3& angles)
    {
        float tmp, yaw, pitch;

        if (forward.y == 0 && forward.x == 0)
        {
            yaw = 0;

            if (forward.z > 0)
                pitch = 270;
            else
                pitch = 90;
        }

        else
        {
            yaw = RAD2DEG(atan2f(forward.y, forward.x));

            if (yaw < 0)
                yaw += 360;

            tmp = forward.Length2D();
            pitch = RAD2DEG(atan2f(-forward.z, tmp));

            if (pitch < 0)
                pitch += 360;
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
    }

    inline float Sigmoid(float x)
    {
        return 1.0f / (1.0f + std::exp(-x));
    }

    inline void AngleVectors(const Vec3& angles, Vec3* forward)
    {
        float sp, sy, cp, cy;

        SinCos(DEG2RAD(angles.x), &sp, &cp);
        SinCos(DEG2RAD(angles.y), &sy, &cy);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }
    }

    inline void AngleVectors(const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up)
    {
        float sr, sp, sy, cr, cp, cy;
        SinCos(DEG2RAD(angles.x), &sp, &cp);
        SinCos(DEG2RAD(angles.y), &sy, &cy);
        SinCos(DEG2RAD(angles.z), &sr, &cr);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
            right->y = (-1 * sr * sp * sy + -1 * cr * cy);
            right->z = -1 * sr * cp;
        }

        if (up)
        {
            up->x = (cr * sp * cy + -sr * -sy);
            up->y = (cr * sp * sy + -sr * cy);
            up->z = cr * cp;
        }
    }

    inline Vec3 CalcAngle(const Vec3& source, const Vec3& destination, bool clamp = true)
    {
        Vec3 angles = {};
        Vec3 delta = source - destination;
        float fHyp = std::sqrtf((delta.x * delta.x) + (delta.y * delta.y));

        angles.x = (atanf(delta.z / fHyp) * M_RADPI);
        angles.y = (atanf(delta.y / delta.x) * M_RADPI);
        angles.z = 0.0f;

        if (delta.x >= 0.0f)
            angles.y += 180.0f;

        if (clamp)
            ClampAngles(angles);

        return angles;
    }

    inline float CalcFov(const Vec3& src, const Vec3& dst)
    {
        Vec3 delta = dst - src;

        NormalizeAngles(delta);

        float result = sqrtf(delta.x * delta.x + delta.y * delta.y);

        // CORREÇÃO: Removido std::isfinite/std::isinf/std::isnan
        if (!isfinite(result) || isinf(result) || isnan(result))
            result = 0.0f;

        return result;
    }

    inline void VectorTransform(const Vec3& input, const matrix3x4_t& matrix, Vec3& output)
    {
        for (auto i = 0; i < 3; i++)
            output[i] = input.Dot((Vec3&)matrix[i]) + matrix[i][3];
    }

    inline float RemapValClamped(float val, float A, float B, float C, float D)
    {
        if (A == B)
            return val >= B ? D : C;

        float cVal = (val - A) / (B - A);
        // CORREÇÃO: Substituindo std::clamp por lógica manual
        cVal = (cVal < 0.0f) ? 0.0f : (cVal > 1.0f) ? 1.0f : cVal;

        return C + (D - C) * cVal;
    }

    inline Vec3 VelocityToAngles(const Vec3& direction)
    {
        auto Magnitude = [&](const Vec3& v) -> float {
            return sqrtf(v.Dot(v));
            };

        float yaw, pitch;

        if (direction.y == 0.0f && direction.x == 0.0f)
        {
            yaw = 0.0f;

            if (direction.z > 0.0f)
                pitch = 270.0f;

            else pitch = 90.0f;
        }

        else
        {
            yaw = RAD2DEG(atan2f(direction.y, direction.x));
            pitch = RAD2DEG(atan2f(-direction.z, Magnitude(Vec3(direction))));

            if (yaw < 0.0f)
                yaw += 360.0f;

            if (pitch < 0.0f)
                pitch += 360.0f;
        }

        return { pitch, yaw, 0.0f };
    }

    inline void MatrixSetColumn(const Vec3& in, int column, matrix3x4_t& out)
    {
        out[0][column] = in.x;
        out[1][column] = in.y;
        out[2][column] = in.z;
    }

    inline void AngleMatrix(const Vec3& angles, matrix3x4_t& matrix)
    {
        float sr, sp, sy, cr, cp, cy;

        SinCos(DEG2RAD(angles[1]), &sy, &cy);
        SinCos(DEG2RAD(angles[0]), &sp, &cp);
        SinCos(DEG2RAD(angles[2]), &sr, &cr);

        matrix[0][0] = cp * cy;
        matrix[1][0] = cp * sy;
        matrix[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;

        matrix[0][1] = sp * srcy - crsy;
        matrix[1][1] = sp * srsy + crcy;
        matrix[2][1] = sr * cp;

        matrix[0][2] = (sp * crcy + srsy);
        matrix[1][2] = (sp * crsy - srcy);
        matrix[2][2] = cr * cp;

        matrix[0][3] = 0.0f;
        matrix[1][3] = 0.0f;
        matrix[2][3] = 0.0f;
    }

    inline void MatrixAngles(const matrix3x4_t& matrix, float* angles)
    {
        float forward[3] = {};
        float left[3] = {};
        float up[3] = {};

        forward[0] = matrix[0][0];
        forward[1] = matrix[1][0];
        forward[2] = matrix[2][0];

        left[0] = matrix[0][1];
        left[1] = matrix[1][1];
        left[2] = matrix[2][1];

        up[2] = matrix[2][2];

        const float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

        if (xyDist > 0.001f)
        {
            angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));
            angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));
            angles[2] = RAD2DEG(atan2f(left[2], up[2]));
        }

        else
        {
            angles[1] = RAD2DEG(atan2f(-left[0], left[1]));
            angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));
            angles[2] = 0.0f;
        }
    }

    inline void RotateTriangle(std::array<Vec2, 3>& points, float rotation)
    {
        Vec2 points_center = (points[0] + points[1] + points[2]) / 3;

        for (auto& point : points)
        {
            point -= points_center;
            float temp_x = point.x;
            float temp_y = point.y;
            float theta = DEG2RAD(rotation);
            float c = cosf(theta);
            float s = sinf(theta);
            point.x = temp_x * c - temp_y * s;
            point.y = temp_x * s + temp_y * c;
            point += points_center;
        }
    }

    inline bool RayToOBB(const  Vec3& origin, const  Vec3& direction, const Vec3& position, const Vec3& min, const Vec3& max, const matrix3x4_t orientation)
    {
        Vec3 p = position - origin;

        Vec3 X = { orientation[0][0], orientation[0][1], orientation[0][2] };
        Vec3 Y = { orientation[1][0], orientation[1][1], orientation[1][2] };
        Vec3 Z = { orientation[2][0], orientation[2][1], orientation[2][2] };

        Vec3 f = { X.Dot(direction), Y.Dot(direction), Z.Dot(direction) };
        Vec3 e = { X.Dot(p), Y.Dot(p), Z.Dot(p) };

        float t[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        for (int i = 0; i < 3; ++i)
        {
            if (floatCompare(f[i], 0.0f))
            {
                if (-e[i] + min[i] > 0.0f || -e[i] + max[i] < 0.0f)
                    return false;

                f[i] = 0.00001f;
            }

            t[i * 2 + 0] = (e[i] + max[i]) / f[i];
            t[i * 2 + 1] = (e[i] + min[i]) / f[i];
        }

        float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
        float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

        if (tmax < 0.0f)
            return false;

        if (tmin > tmax)
            return false;

        return true;
    }

    inline void VectorRotate(Vec3& in1, const matrix3x4_t& in2, Vec3& out)
    {
        out[0] = in1.Dot(in2[0]);
        out[1] = in1.Dot(in2[1]);
        out[2] = in1.Dot(in2[2]);
    }
}

#pragma warning (pop)