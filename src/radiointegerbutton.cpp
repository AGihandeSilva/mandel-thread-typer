#include "radiointegerbutton.h"
#include "renderthread.h"
#include "buttonuser.h"

RadioIntegerButton::RadioIntegerButton()
    : value (0), receiver(nullptr) {}

RadioIntegerButton::RadioIntegerButton(int value, ButtonUser *receiver)
    : QRadioButton(QString::number(value)), value(value), receiver(receiver) {}

void RadioIntegerButton::enforceValue()
{
    receiver->processIntegerValueFromButtonPress(value);
}
