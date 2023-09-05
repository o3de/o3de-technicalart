#pragma once

#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>

#include <HitContext.h>
//O3DECONVERT
//#include <EditTool.h>
#include <Viewport.h>

namespace HoudiniEngine
{
    static const float kClickDistanceScaleFactor = 0.01f;
    static const float kSplinePointSelectionRadius = 0.8f;

    class HoudiniPaintAttribTool
        //: public CEditTool
    {
    public:
        // Overrides from CEditTool
        //bool MouseCallback(CViewport* view, EMouseEvent event, QPoint& point, int flags) override;
        //void OnManipulatorDrag(CViewport* pView, ITransformManipulator* pManipulator, QPoint& point0, QPoint& point1, const Vec3& value) override;
        
        //void Display(DisplayContext& dc) override;
        //bool OnKeyDown(CViewport* view, uint32 nChar, uint32 nRepCnt, uint32 nFlags) override;

        void SetSplines(AZStd::vector<AZ::EntityId>& splines);

        //bool IsNeedMoveTool() override { return true; }
        //bool IsNeedToSkipPivotBoxForObjects() override { return true; }

        void SetAttributeName(const AZStd::string& name)
        {
            m_name = name;
        }

        void SetRadius(float value)
        {
            m_radius = value;
        }

    protected:
        virtual ~HoudiniPaintAttribTool();
        void DeleteThis() { delete this; }
        void SetCursor(EStdCursor cursor, bool bForce = false);
                
        AZStd::string m_name;
        float m_radius = 10.0f;

        AZStd::vector<AZ::EntityId> m_splines;
        AZ::Vector3 m_hoverPoint;
        AZ::ConstSplinePtr m_hoverSpline = nullptr;
        AZ::EntityId m_hoverId;
        AZ::SplineAddress m_addressPoint;
        bool m_painting = false;

        bool m_showDetails = false;

    };

}
