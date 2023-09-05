#pragma once

#if !defined(Q_MOC_RUN)
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <QDialog>
#include <QMainWindow>
#endif

class QString;
class QLineEdit;
class QStringListModel;

namespace Ui 
{
    class HoudiniStatusPanel;
}

namespace HoudiniEngine
{
    class HoudiniStatusPanel
        : public QWidget
    {
        Q_OBJECT
    public:

        HoudiniStatusPanel();
        virtual ~HoudiniStatusPanel();
        
        void UpdatePercent(AZStd::string text, int percent, int assetsInQueue, bool asyncMode);

        AZStd::function<void()> OnInterrupt = {};

    private slots:
        

    private:        
        void OnClickInterrupt();

        Ui::HoudiniStatusPanel* m_ui;
    };
}
