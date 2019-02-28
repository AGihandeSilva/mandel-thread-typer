#ifndef BUTTONUSER_H
#define BUTTONUSER_H

class ButtonUser
{
public:
    virtual ~ButtonUser() = default;
    virtual void processIntegerValueFromButtonPress(int value) = 0;
};

#endif // BUTTONUSER_H
