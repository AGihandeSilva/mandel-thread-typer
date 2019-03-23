#include "MandelbrotGuiTools.h"

#include <QWidget>
#include <QFrame>
#include <QBoxLayout>

namespace MandelBrotRenderer
{
    void addHorizontalLine(QWidget* owner, QBoxLayout *layout)
    {
        const int spacingAroundHLines = 20;

        layout->addSpacing(spacingAroundHLines);

        QFrame* frame = new QFrame(owner);
        frame->setFrameShape(QFrame::HLine);
        frame->setFrameShadow(QFrame::Sunken);
        layout->addWidget(frame);

        layout->addSpacing(spacingAroundHLines);
    }
}
