#include "StdAfx.h"

#include <Source/ui_HoudiniStatusPanel.h>
#include "HoudiniStatusPanel.h"

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
//#include <QWebView>  // FL[FD-12916] Lumberyard 1.23 integration

#include <QPushButton>
#include <QStringListModel>


namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    HoudiniStatusPanel::HoudiniStatusPanel()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniStatusPanel())
    {
        m_ui->setupUi(this);
        
        //TODO: This doesn't seem to work atm:
        //connect(m_ui->cmdInterrupt, &QPushButton::clicked, this, &HoudiniStatusPanel::OnClickInterrupt);
        
    }
    
    HoudiniStatusPanel::~HoudiniStatusPanel()
    {

    }
    
    void HoudiniStatusPanel::OnClickInterrupt()
    {
        if (OnInterrupt)
        {
            OnInterrupt();
        }
    }

    void HoudiniStatusPanel::UpdatePercent(AZStd::string text, int percent, int assetsInQueue, bool asyncMode)
    {
        if (m_ui->progress->value() != percent)
        {
            if (percent < 0)
            {
                percent = 0;
            }

            m_ui->progress->setValue(percent);
        }

        if (!asyncMode)
        {
            if (m_ui->txtHoudiniMode->text() != "Houdini: Embedded")
            {
                m_ui->txtHoudiniMode->setText("Houdini: Embedded");
            }            
        }
        else
        {
            if (m_ui->txtHoudiniMode->text() != "Houdini: HARS.exe")
            {
                m_ui->txtHoudiniMode->setText("Houdini: HARS.exe");
            }
        }

        QString newDesc = QString(text.c_str());
        QString remainder = QString("Queue: ") + QString::number(assetsInQueue);

        if (percent != 0)
        {
            newDesc += " " + QString::number(percent) + "%";
        }

        if (m_ui->txtTasksRemaining->text() != remainder)
        {
            m_ui->txtTasksRemaining->setText(remainder);
        }

        if (m_ui->txtLabel->text() != newDesc)
        {
            m_ui->txtLabel->setText(newDesc);
        }
    }
}

#include <moc_HoudiniStatusPanel.cpp>
