#include "StdAfx.h"

#include "OperatorSelection.h"

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
//O3DECONVERT
//#include <QWebEngineSettings> 
//#include <QWebEngineView> 

#include <QPushButton>
#include <QStringListModel>

#include <AzCore/XML/rapidxml.h>

#include <Source/ui_OperatorSelection.h>

#include "AssetContainer.h"

namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    OperatorSelection::OperatorSelection(OperatorMode selectionMode, const AZStd::string& previousValue)
        : QDialog(nullptr)
        , m_selectionMode(selectionMode)
        , m_previousValue(previousValue)
        , m_ui(new Ui::OperatorSelection())
    {
        m_ui->setupUi(this);
        
        m_ui->listOperators->installEventFilter(this);
        m_ui->listOperators->verticalScrollBar()->installEventFilter(this);

        m_ui->listOperators->verticalScrollBar()->setPageStep(1);
        m_ui->listOperators->verticalScrollBar()->setSingleStep(1);
        
        LoadData();

        connect(m_ui->cmdButtons, SIGNAL(accepted()), this, SLOT(accept()));
        connect(m_ui->cmdButtons, SIGNAL(rejected()), this, SLOT(reject()));
        
        connect(m_ui->listOperators, &QListWidget::currentItemChanged, this, &OperatorSelection::OnSelectionChanged);
        connect(m_ui->listOperators, &QListWidget::itemDoubleClicked, this, &OperatorSelection::OnSelectionDoubleClick);
    }

    OperatorSelection::~OperatorSelection()
    {

    }

    bool OperatorSelection::eventFilter(QObject *obj, QEvent *event)
    {
        static int jumpSize = 15;

        if (event != nullptr && event->type() == QEvent::Wheel)
        {
            if (obj == m_ui->listOperators || obj == m_ui->listOperators->verticalScrollBar())
            {
                QWheelEvent *wEvent = (QWheelEvent *)event;

                QScrollBar * vScroll = m_ui->listOperators->verticalScrollBar();

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

    void OperatorSelection::OnSelectionChanged(QListWidgetItem *current, QListWidgetItem */*previous*/)
    {
        AssetContainer container = current->data(Qt::UserRole).value<AssetContainer>();
        auto* asset = container.asset;

        QGroupBox * webBox = m_ui->webBox;
        QLayout* layout = webBox->layout();

        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != 0)
        {
            delete child->widget();
            delete child;
        }
        layout->invalidate();

        if (asset)
        {
            QString imagePath = asset->GetHdaFile().c_str();
            imagePath = imagePath.toLower();
            imagePath = imagePath.replace(".hda", ".html");
            
            //TODO
            // O3DECONVERT
            /*if (QFile::exists(imagePath))
            {
                QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

                QWebEngineView* view = new QWebEngineView();
                view->setUrl("file:///" + imagePath);
                //view->page()->setLinkDelegationPolicy(QWebEnginePage::DelegateAllLinks);
                //connect(view, &QWebEngineView::linkClicked, this, &OperatorSelection::OnLinkClicked);

                layout->addWidget(view);
            }*/
        }

        m_selectedOperator = current->text().toUtf8().data();
    }

    void OperatorSelection::OnSelectionDoubleClick(QListWidgetItem *item)
    {
        if (item != nullptr)
        {
            OnSelectionChanged(item, item);
            accept();
        }
    }

    void OperatorSelection::OnLinkClicked(const QUrl& /*item*/)
    {
        //TODO
        //O3DECONVERT
        //QDesktopServices::openUrl(item);
    }

    void OperatorSelection::LoadData()
    {
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        AZStd::vector<AZStd::string> output;
        output.push_back("");

        if (hou != nullptr && hou->IsActive())
        {
            hou->ReloadAllAssets(); 
            auto assets = hou->GetAvailableAssets();

            for (auto asset : assets)
            {
                auto result = asset->getAssets();

                if (m_selectionMode == OperatorMode::Scatter)
                {
                    QString file = asset->GetHdaFile().c_str();
                    file = file.toLower();                    

                    if (file.indexOf("scatter") >= 0)
                    {
                        for (auto op : result)
                        {
                            AddAsset(asset.get(), op);
                        }
                    }
                }
                else if (m_selectionMode == OperatorMode::Terrain)
                {
                    QString file = asset->GetHdaFile().c_str();
                    file = file.toLower();
                    if (file.indexOf("terrain") >= 0)
                    {
                        for (auto op : result)
                        {
                            AddAsset(asset.get(), op);
                        }
                    }
                }
                else
                {
                    for (auto op : result)
                    {
                        AddAsset(asset.get(), op);
                    }
                }
            }
        }
    }

    void OperatorSelection::AddAsset(IHoudiniAsset* asset, const AZStd::string& operatorName)
    {
        QString imagePath = asset->GetHdaFile().c_str();
        imagePath = imagePath.toLower();
        imagePath = imagePath.replace(".hda", ".png");

        QString configPath = asset->GetHdaFile().c_str();
        configPath = configPath.toLower();
        configPath = configPath.replace(".hda", ".xml");

        bool canShow = true;

        if (QFile::exists(configPath))
        {
            QFile file(configPath);
            if (file.open(QIODevice::ReadOnly))
            {
                QTextStream in(&file);
                QString docSrc = in.readAll();
                file.close();

                using namespace AZ::rapidxml;
                xml_document<> doc;
                if (doc.parse<0>(docSrc.toUtf8().data()))
                {
                    xml_node<>* cur_node = doc.first_node("HDA", 0, false);
                    if (cur_node)
                    {
                        xml_attribute<>* attrib = cur_node->first_attribute("ShowInSelector", 0, false);
                        if (attrib)
                        {
                            if (QString("false").compare(attrib->value(), Qt::CaseInsensitive) == 0)
                            {
                                canShow = false;
                            }
                        }

                        attrib = cur_node->first_attribute("HideInSelector", 0, false);
                        if (attrib)
                        {
                            QString hideList = attrib->value();
                            QStringList hideParts = hideList.split("[ ,]");

                            for (const auto& hidePart : hideParts)
                            {
                                if (hidePart.compare(operatorName.c_str(), Qt::CaseInsensitive) == 0)
                                {
                                    canShow = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (canShow == false)
        {
            return;
        }

        QString previewPath = "://Icons/UnknownPreview.png";
        if (QFile::exists(imagePath))
        {
            previewPath = imagePath;
        }

        AssetContainer container;
        container.asset = asset;

        QVariant userData = QVariant::fromValue(container);
        
        QListWidgetItem * item = new QListWidgetItem(m_ui->listOperators);
        QIcon icon(previewPath);
        item->setText(operatorName.c_str());
        item->setIcon(icon);
        item->setData(Qt::UserRole, userData);
        item->setSizeHint(QSize(255,200));

        if (m_previousValue == operatorName) 
        {
            m_ui->listOperators->setCurrentItem(item);
        }
    }

    //////////////////////////////////////////////////////////////////////////

}

#include <moc_OperatorSelection.cpp>
