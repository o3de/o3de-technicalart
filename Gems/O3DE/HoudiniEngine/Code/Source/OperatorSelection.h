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
    class OperatorSelection;
}

namespace HoudiniEngine
{
    class IHoudiniAsset;

    class OperatorSelection
        : public QDialog
    {
        Q_OBJECT
    public:

        OperatorSelection(OperatorMode selectionMode, const AZStd::string& m_previousValue = "");
        virtual ~OperatorSelection();
       
        const AZStd::string& GetSelectedOperator() const
        {
            return m_selectedOperator;
        }

    private slots:
        void OnSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
        void OnSelectionDoubleClick(QListWidgetItem *item);
        void OnLinkClicked(const QUrl& item);
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        void LoadData();
        void AddAsset(HoudiniEngine::IHoudiniAsset* asset, const AZStd::string& operatorName);

        AZStd::string m_selectedOperator;
        OperatorMode m_selectionMode = OperatorMode::Assets;
        
        AZStd::string m_previousValue;

        Ui::OperatorSelection* m_ui;
    };
}
