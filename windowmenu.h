#ifndef WINDOWMENU_H
#define WINDOWMENU_H

#include <QMenu>

class WindowThreadInfo;
class RenderThread;
class ThreadIconMap;
class WindowThreadInfoKey;
class QSize;
class QPoint;

class WindowMenu : public QMenu
{
    Q_OBJECT
public:
    explicit WindowMenu(RenderThread* masterThread);

    const ThreadIconMap *getThreadIcons() const;

    QSize getThreadInfoWindowSize() const;
    QPoint getThreadInfoWindowPos() const;

public slots:
    void showInfo();
    void showKey();

private:
    WindowThreadInfo* threadInfo;
    RenderThread* masterThread;
    const ThreadIconMap* threadIcons;
    WindowThreadInfoKey* threadStateKey;
};

#endif // WINDOWMENU_H
