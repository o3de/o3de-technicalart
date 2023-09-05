#ifndef H_ENGINE_PARAMETERWIDGET_STRINGCHOICE
#define H_ENGINE_PARAMETERWIDGET_STRINGCHOICE

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>
#include <string>
#include <vector>
#endif

class QGridLayout;
class QLabel;
class QComboBox;

namespace HoudiniEngine
{
    class HE_ParameterWidget_StringChoice : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_StringChoice() = delete;
        HE_ParameterWidget_StringChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                        int ChoiceSize,
                                        const std::vector<std::string> &Choices,
                                        const std::vector<std::string> &ChoiceVals,
                                        std::string CurrentChoice);
        ~HE_ParameterWidget_StringChoice();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        QComboBox *StringChoices;

        std::vector<std::string> Values;

    private slots:
        void CurrentIndexChanged(int Index);

    signals:
        void Signal_StringChoiceParmUpdate(HAPI_ParmId, std::string);
    };
}

#endif
