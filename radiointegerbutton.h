#ifndef RADIOINTEGERBUTTON_H
#define RADIOINTEGERBUTTON_H

#include <QRadioButton>

class RenderThread;
class ButtonUser;

class RadioIntegerButton : public QRadioButton
{
    Q_OBJECT
public:
    RadioIntegerButton();
    explicit RadioIntegerButton(int value, ButtonUser* receiver);
    int getValue() const { return value; }

private slots:
    void enforceValue();


private:
    const int value;
    ButtonUser* const receiver;
};

#endif // RADIOINTEGERBUTTON_H
