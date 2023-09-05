
#include "StdAfx.h"

#include <HoudiniCommon.h>
#include "HoudiniPaintAttribTool.h"

#include <Objects/DisplayContext.h>
#include "Util/EditorUtils.h"

namespace HoudiniEngine
{

    HoudiniPaintAttribTool::~HoudiniPaintAttribTool()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    void HoudiniPaintAttribTool::SetSplines(AZStd::vector<AZ::EntityId>& splines)
    {
        m_splines.clear();

        for (auto id : splines)
        {
            m_splines.push_back(id);
        }
    }

    /*bool HoudiniPaintAttribTool::MouseCallback(CViewport* view, EMouseEvent event, QPoint& point, int flags)
    {

        Vec3 raySrc, rayDir;
        view->ViewToWorldRay(point, raySrc, rayDir);

        AZ::Vector3 vRaySrc(raySrc.x, raySrc.y, raySrc.z);
        AZ::Vector3 vRayDir(rayDir.x, rayDir.y, rayDir.z);

        // Find closest point on the splines
        float distance = 1e10f;                
        AZ::EntityId nearestSpline;
        AZ::RaySplineQueryResult nearestHit;

        for (auto id : m_splines)
        {
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, id, &AZ::TransformBus::Events::GetWorldTM);
            AZ::Transform invertedTransform = transform.GetInverseFull();

            AZ::ConstSplinePtr spline = nullptr;
            LmbrCentral::SplineComponentRequestBus::EventResult(spline, id, &LmbrCentral::SplineComponentRequests::GetSpline);
            if (spline != nullptr)
            {                
                AZ::Vector3 localPosition = invertedTransform * vRaySrc;
                AZ::Vector3 localRot = invertedTransform.Multiply3x3(vRayDir);

                auto result = spline->GetNearestAddressRay(localPosition, localRot);
                
                if (result.m_rayDistance < distance) 
                {
                    distance = result.m_rayDistance;
                    nearestHit = result;
                    nearestSpline = id;
                    m_hoverId = id;
                    m_addressPoint = nearestHit.m_splineAddress;
                    m_hoverSpline = spline;
                }
            }
        }

        if (event == eMouseLDown)
        {
            m_painting = true;
        }
        else if (event == eMouseLUp)
        {
            m_painting = false;
            HoudiniCurveAttributeRequestBus::Event(nearestSpline, &HoudiniCurveAttributeRequests::CommitChanges);
        }

        AZ::ConstSplinePtr spline = nullptr;
        LmbrCentral::SplineComponentRequestBus::EventResult(spline, nearestSpline, &LmbrCentral::SplineComponentRequests::GetSpline);
        if (spline != nullptr)
        {
            GetIEditor()->BeginUndo();
                        
            float value = 0.0f;
            HoudiniCurveAttributeRequestBus::Event(nearestSpline, &HoudiniCurveAttributeRequests::GetPaintValue, m_name, value);
                
            int pos = (int)nearestHit.m_splineAddress.m_segmentIndex;
            if (nearestHit.m_splineAddress.m_segmentFraction > 0.5f)
            {
                pos++;
            }
            
            AZ::Vector3 vEditPoint = spline->GetPosition(AZ::SplineAddress(pos, 0));
            m_hoverPoint = vEditPoint;
            
            if (m_painting)
            {
                HoudiniCurveAttributeRequestBus::Event(nearestSpline, &HoudiniCurveAttributeRequests::SetValue, m_name, pos, value);
            }

            if (GetIEditor()->IsUndoRecording())
            {
                GetIEditor()->AcceptUndo("Spline Paint Attribute");
            }
        }

        return true;
    }

    void HoudiniPaintAttribTool::OnManipulatorDrag(CViewport* pView, ITransformManipulator* pManipulator, QPoint& point0, QPoint& point1, const Vec3& value)
    {

    }

    void HoudiniPaintAttribTool::Display(DisplayContext& dc)
    {        
        if (m_hoverSpline != nullptr)
        {
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, m_hoverId, &AZ::TransformBus::Events::GetWorldTM);
            const AZ::Transform& invertedTransform = transform.GetInverseFull();

            int pos = (int)m_addressPoint.m_segmentIndex;
            if (m_addressPoint.m_segmentFraction > 0.5f)
            {
                pos++;
            }

            int count = 0;
            HoudiniCurveAttributeRequestBus::Event(m_hoverId, &HoudiniCurveAttributeRequests::GetValueCount, m_name, count);

            float minVal = 0.0f;
            float maxVal = 1.0f;
            HoudiniCurveAttributeRequestBus::Event(m_hoverId, &HoudiniCurveAttributeRequests::GetValueRange, m_name, minVal, maxVal);

            m_showDetails = CheckVirtualKey(Qt::Key_Shift);

            if (count > 0)
            {
                Vec3 pPosition = AZVec3ToLYVec3(transform * m_hoverSpline->GetPosition(AZ::SplineAddress(0, 0)));
                float pPercent;

                for (int i = 0; i < count; i++)
                {
                    float value = 0.0f;
                    HoudiniCurveAttributeRequestBus::Event(m_hoverId, &HoudiniCurveAttributeRequests::GetValue, m_name, i, value);

                    float percent = 1.0f;
                    if (maxVal - minVal != 0.0f)
                    {
                        percent = (value - minVal) / (maxVal - minVal);

                        if (i == 0) 
                            pPercent = percent;
                    }

                    const AZ::Vector3& localPos = m_hoverSpline->GetPosition(AZ::SplineAddress(i, 0));
                    AZ::Vector3 vPosition = transform * localPos;
                    Vec3 position(vPosition.GetX(), vPosition.GetY(), vPosition.GetZ());

                    static float lineWidth = 50;
                    dc.SetLineWidth(lineWidth);
                    dc.SetColor(percent, percent, percent);
                    ColorF color1 = ColorF(pPercent, pPercent, pPercent);
                    ColorF color2 = ColorF(percent, percent, percent);
                    
                    dc.DrawLine(pPosition, position, color1, color2);
                    
                    pPercent = percent;
                    pPosition = position;

                    if (m_showDetails || i == pos)
                    {
                        if (abs(i - pos) < 5)
                        {
                            AZStd::string indexFormat = AZStd::string::format("[%.2f]", value);

                            if (gEnv && gEnv->pRenderer)
                            {
                                float sx = 0.0f, sy = 0.0f, sz = 0.0f;

                                gEnv->pRenderer->ProjectToScreen(position.x, position.y, position.z, &sx, &sy, &sz);
                                sx *= gEnv->pRenderer->GetWidth() * 0.01f;
                                sy *= gEnv->pRenderer->GetHeight() * 0.01f;
                                sy += 40.0f;

                                dc.Draw2dTextLabel(sx, sy, 1.5f, indexFormat.c_str(), true);
                            }
                            else
                            {
                                dc.DrawTextLabel(position, 1.5f, indexFormat.c_str(), true);
                            }
                        }
                    }
                }
            }
        }
    }

    bool HoudiniPaintAttribTool::OnKeyDown(CViewport* view, uint32 nChar, uint32 nRepCnt, uint32 nFlags)
    {
        if (nChar == VK_ESCAPE)
        {
            GetIEditor()->SetEditTool(0);
        }

        
        return true;
    }*/

    void HoudiniPaintAttribTool::SetCursor(EStdCursor /*cursor*/, bool /*bForce = false*/)
    {
        
    }

}
