#include "StdAfx.h"
//#include "..\..\BinTemp\win_x64_vs2015_profile\qt5\EzStreet.Editor\Source\ui_EzStreetMainWindow.h"  //TEMP So I can see intellisense

#include "ExtractPoints.h"

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include "Components/HoudiniNodeComponentConfig.h"
#include <HoudiniCommon.h>

#include <QtUtilWin.h>
#include <QFile>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QSettings>
#include <QWheelEvent>
#include <QScrollBar>

// #include <QWebView> // FL[FD-12916] Lumberyard 1.23 integration

#include <QPushButton>
#include <QStringListModel>

#include <Source/ui_ExtractPoints.h>

#include "AssetContainer.h"

namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    ExtractPoints::ExtractPoints(IHoudiniNode* node)
        : QDialog(nullptr)
        , m_node(node)
        , m_ui(new Ui::ExtractPoints())
    {
        m_ui->setupUi(this);
        
        m_ui->listGroups->installEventFilter(this);
        m_ui->listGroups->verticalScrollBar()->installEventFilter(this);

        m_ui->listGroups->verticalScrollBar()->setPageStep(1);
        m_ui->listGroups->verticalScrollBar()->setSingleStep(1);
        
        LoadData();

        connect(m_ui->cmdButtons, SIGNAL(accepted()), this, SLOT(accept()));
        connect(m_ui->cmdButtons, SIGNAL(rejected()), this, SLOT(reject()));
        
        connect(m_ui->listGroups, &QListWidget::itemSelectionChanged, this, &ExtractPoints::OnSelectionChanged);
        connect(m_ui->listGroups, &QListWidget::itemDoubleClicked, this, &ExtractPoints::OnSelectionDoubleClick);

        connect(m_ui->chkUseChildren, &QCheckBox::stateChanged, this, &ExtractPoints::OnSelectionChanged);
    }

    ExtractPoints::~ExtractPoints()
    {

    }

    bool ExtractPoints::eventFilter(QObject *obj, QEvent *event)
    {
        static int jumpSize = 15;

        if (event != nullptr && event->type() == QEvent::Wheel)
        {
            if (obj == m_ui->listGroups || obj == m_ui->listGroups->verticalScrollBar())
            {
                QWheelEvent *wEvent = (QWheelEvent *)event;

                QScrollBar * vScroll = m_ui->listGroups->verticalScrollBar();

                if (wEvent->angleDelta().y() > 0)
                {
                    vScroll->setValue(vScroll->value() - jumpSize);
                }
                else
                {
                    vScroll->setValue(vScroll->value() + jumpSize);
                }
                return true;
            }
        }

        return false;
    }

    void ExtractPoints::OnSelectionChanged()
    {
        m_selectedGroups.clear();
        for (auto* item : m_ui->listGroups->selectedItems())
        {
            m_selectedGroups.push_back(item->text().toUtf8().data());
        }

        m_reuseChildren = m_ui->chkUseChildren->isChecked();
    }

    void ExtractPoints::OnSelectionDoubleClick(QListWidgetItem *item)
    {
        if (item != nullptr)
        {
            OnSelectionChanged();
            accept();
        }
    }

    void ExtractPoints::LoadData()
    {
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        AZStd::vector<AZStd::string> output;
        output.push_back("");

        if (hou != nullptr && hou->IsActive())
        {
            HAPI_GeoInfo geoInfo = m_node->GetGeometryInfo();
            AZStd::vector<HAPI_StringHandle> groupNames(geoInfo.pointGroupCount);
            HAPI_GetGroupNames(&hou->GetSession(), m_node->GetId(), HAPI_GROUPTYPE_POINT, groupNames.data(), geoInfo.pointGroupCount);
            
            for (auto& stringHandle : groupNames)
            {
                QListWidgetItem * item = new QListWidgetItem(m_ui->listGroups);
                item->setText(hou->GetString(stringHandle).c_str());
            }
        }
    }
    
    //////////////////////////////////////////////////////////////////////////

}

#include <moc_extractpoints.cpp>
