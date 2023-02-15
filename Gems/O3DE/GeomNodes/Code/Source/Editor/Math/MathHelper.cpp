#include <Editor/Math/MathHelper.h>

namespace GeomNodes
{
    constexpr auto SMALL_NUMBER = (1.e-8f);
    AZ::Vector3 MathHelper::ExtractScalingFromMatrix44(AZ::Matrix4x4& m)
    {
        AZ::Vector3 Scale3D(0, 0, 0);

        // For each row, find magnitude, and if its non-zero re-scale so its unit length.
        const float SquareSum0 = (m.GetElement(0, 0) * m.GetElement(0, 0)) + (m.GetElement(0, 1) * m.GetElement(0, 1)) +
            (m.GetElement(0, 2) * m.GetElement(0, 2));
        const float SquareSum1 = (m.GetElement(1, 0) * m.GetElement(1, 0)) + (m.GetElement(1, 1) * m.GetElement(1, 1)) +
            (m.GetElement(1, 2) * m.GetElement(1, 2));
        const float SquareSum2 = (m.GetElement(2, 0) * m.GetElement(2, 0)) + (m.GetElement(2, 1) * m.GetElement(2, 1)) +
            (m.GetElement(2, 2) * m.GetElement(2, 2));

        if (SquareSum0 > SMALL_NUMBER)
        {
            float Scale0 = AZ::Sqrt(SquareSum0);
            Scale3D.SetElement(0, Scale0);
            float InvScale0 = 1.f / Scale0;
            m.SetElement(0, 0, m.GetElement(0, 0) * InvScale0);
            m.SetElement(0, 1, m.GetElement(0, 1) * InvScale0);
            m.SetElement(0, 2, m.GetElement(0, 2) * InvScale0);
        }
        else
        {
            Scale3D.SetElement(0, 0);
        }

        if (SquareSum1 > SMALL_NUMBER)
        {
            float Scale1 = AZ::Sqrt(SquareSum1);
            Scale3D.SetElement(1, Scale1);
            float InvScale1 = 1.f / Scale1;

            m.SetElement(1, 0, m.GetElement(1, 0) * InvScale1);
            m.SetElement(1, 1, m.GetElement(1, 1) * InvScale1);
            m.SetElement(1, 2, m.GetElement(1, 2) * InvScale1);
        }
        else
        {
            Scale3D.SetElement(1, 0);
        }

        if (SquareSum2 > SMALL_NUMBER)
        {
            float Scale2 = AZ::Sqrt(SquareSum2);
            Scale3D.SetElement(2, Scale2);
            float InvScale2 = 1.f / Scale2;

            m.SetElement(2, 0, m.GetElement(2, 0) * InvScale2);
            m.SetElement(2, 1, m.GetElement(2, 1) * InvScale2);
            m.SetElement(2, 2, m.GetElement(2, 2) * InvScale2);
        }
        else
        {
            Scale3D.SetElement(2, 0);
        }

        return Scale3D;
    }
        
    AZ::Matrix4x4 MathHelper::ConvertTransformAndScaleToMat4(const AZ::Transform& transform, const AZ::Vector3& nonUniformScale)
    {
        const AZ::Vector3& o3deTranslation = transform.GetTranslation();
        const AZ::Quaternion& o3deRotation = transform.GetRotation();
        AZ::Vector3 newScale = transform.GetUniformScale() * nonUniformScale;

        AZ::Matrix4x4 newTransform;
        newTransform.SetTranslation(o3deTranslation);
        newTransform.SetRotationPartFromQuaternion(o3deRotation);
        newTransform.MultiplyByScale(newScale);
        
        return newTransform;
    }

    std::size_t MathHelper::Align(std::size_t location, std::size_t align)
    {
        //AZ_Assert(((0 != align) && !(align & (align - 1))), "non-power of 2 alignment");
        return ((location + (align - 1)) & ~(align - 1));
    }
    AZ::Vector2 MathHelper::Vec2fToVec2(const Vector2f& vec)
    {
        return AZ::Vector2(vec[0], vec[1]);
    }
    AZ::Vector3 MathHelper::Vec3fToVec3(const Vector3f& vec)
    {
        return AZ::Vector3(vec[0], vec[1], vec[2]);
    }
    AZ::Vector4 MathHelper::Vec4fToVec4(const Vector4f& vec)
    {
        return AZ::Vector4(vec[0], vec[1], vec[2], vec[3]);
    }
    Vector2f MathHelper::Vec2ToVec2f(const AZ::Vector2& vec)
    {
        return Vector2f{ vec.GetX(), vec.GetY() };
    }
    Vector3f MathHelper::Vec3ToVec3f(const AZ::Vector3& vec)
    {
        return Vector3f{ vec.GetX(), vec.GetY(), vec.GetZ() };
    }
    Vector4f MathHelper::Vec4ToVec4f(const AZ::Vector4& vec)
    {
        return Vector4f{ vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW() };
    }
}
