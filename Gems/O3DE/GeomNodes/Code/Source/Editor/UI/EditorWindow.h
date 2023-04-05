#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Asset/AssetCommon.h>
#include <QWidget>
#endif

namespace Ui
{
    class EditorWindowClass;
}

namespace GeomNodes
{
    struct GNConfiguration;
    
    namespace Editor
    {
        /// Window pane wrapper for the GeomNodes Configuration Widget.
        ///
        class EditorWindow
            : public QWidget
        {
            Q_OBJECT
        public:
            AZ_CLASS_ALLOCATOR(EditorWindow, AZ::SystemAllocator);
            static void RegisterViewClass();

            explicit EditorWindow(QWidget* parent = nullptr);

        private:
            static void SaveConfiguration(const GeomNodes::GNConfiguration& GNConfiguration);

            QScopedPointer<Ui::EditorWindowClass> m_ui;
        };
    }
};


