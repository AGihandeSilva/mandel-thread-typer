#include "windowthreadinfokey.h"
#include "threadiconmap.h"

#include <QLabel>
#include <QGridLayout>

WindowThreadInfoKey::WindowThreadInfoKey(const ThreadIconMap* threadIcons) : QGroupBox("Thread State Display Key"), threadIcons(threadIcons)
{
    this->setStyleSheet("QGroupBox { font-weight: bold; } ");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    threadStateKeyLayout = new QGridLayout();
    const StateNameMap& stateNames  = threadIcons->getStateNames();

    int counter(0);
    for(const auto& i  : stateNames)
    {
        QLabel* aStateText (new QLabel());
        aStateText->setText(i.second);
        QLabel* aStateIndicator (new QLabel());
        auto index = static_cast<size_t>(toUnderlyingType(i.first));
        aStateIndicator->setPixmap(threadIcons->getThreadBitmaps().at(index));
        aStateIndicator->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        aStateIndicator->setLineWidth(MandelBrotRenderer::ThreadIndicatorFrameWidth);

        threadStateKeyLayout->addWidget(aStateText, counter, 0, Qt::AlignLeft);
        threadStateKeyLayout->addWidget(aStateIndicator, counter, 1, Qt::AlignHCenter);
        counter++;
    }

    setLayout(threadStateKeyLayout);
    resize(QSize(width, height));
}
