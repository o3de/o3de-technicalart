#ifndef H_ENGINE_PARAMETERWIDGET_INTEGERCHOICE
#define H_ENGINE_PARAMETERWIDGET_INTEGERCHOICE

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
    class HE_ParameterWidget_IntegerChoice : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_IntegerChoice() = delete;
        HE_ParameterWidget_IntegerChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                            int ChoiceSize,
                                            const std::vector<std::string> &Choices,
                                            int CurrentChoice);
        ~HE_ParameterWidget_IntegerChoice();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        QComboBox *IntChoices;

    private slots:
        void CurrentIndexChanged(int);

    signals:
        void Signal_IntegerChoiceParmUpdate(HAPI_ParmId, int);
    };
}

#endif
