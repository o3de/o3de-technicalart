#pragma once

#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Matrix4x4.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Utils/TypeHash.h>

namespace GeomNodes
{
    using Vector4f = AZStd::array<float, 4>;
    using Vector3f = AZStd::array<float, 3>;
    using Vector2f = AZStd::array<float, 2>;

    using U32Vector = AZStd::vector<AZ::u32>;
    using S32Vector = AZStd::vector<AZ::s32>;
    using S64Vector = AZStd::vector<AZ::s64>;
    using Vert2Vector = AZStd::vector<Vector2f>;
    using Vert3Vector = AZStd::vector<Vector3f>;
    using Vert4Vector = AZStd::vector<Vector4f>;
    using Mat4Vector = AZStd::vector<AZ::Matrix4x4>;

    using UniqueKey = AZStd::tuple<AZ::s32, Vector3f, Vector2f, Vector4f>;

    struct MathHelper
    {
        static AZ::Vector3 ExtractScalingFromMatrix44(AZ::Matrix4x4& m);
        static AZ::Matrix4x4 ConvertTransformAndScaleToMat4(const AZ::Transform& transform, const AZ::Vector3& nonUniformScale);
        static std::size_t Align(std::size_t location, std::size_t align);
        static AZ::Vector2 Vec2fToVec2(const Vector2f& vec);
        static AZ::Vector3 Vec3fToVec3(const Vector3f& vec);
        static AZ::Vector4 Vec4fToVec4(const Vector4f& vec);
        static Vector2f Vec2ToVec2f(const AZ::Vector2& vec);
        static Vector3f Vec3ToVec3f(const AZ::Vector3& vec);
        static Vector4f Vec4ToVec4f(const AZ::Vector4& vec);
    };
} // namespace GeomNodes

namespace AZStd
{
    template<>
    struct hash<GeomNodes::UniqueKey>
    {
        typedef GeomNodes::UniqueKey argument_type;
        typedef AZStd::size_t result_type;
        AZ_FORCE_INLINE size_t operator()(const GeomNodes::UniqueKey& id) const
        {
            AZStd::hash<AZ::u32> hasher;
            return hasher(static_cast<AZ::u32>(AZ::TypeHash32(id)));
        }
    };
} // namespace AZStd
