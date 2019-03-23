#ifndef BUTTONUSER_H
#define BUTTONUSER_H

class ButtonUser
{
public:
    ButtonUser() = default;
    virtual ~ButtonUser() = default;
    ButtonUser(const ButtonUser&) = default;
    ButtonUser(ButtonUser&&) = default;
    ButtonUser& operator=(const ButtonUser&) = default;
    ButtonUser& operator=(ButtonUser&&) = default;
    virtual void processIntegerValueFromButtonPress(int value) = 0;
};

#endif // BUTTONUSER_H
