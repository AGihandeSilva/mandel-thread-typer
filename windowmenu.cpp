#include "windowmenu.h"
#include "windowthreadinfo.h"
#include "renderthread.h"
#include "threadiconmap.h"
#include "windowthreadinfokey.h"

WindowMenu::WindowMenu(RenderThread* masterThread) : masterThread(masterThread)
{
    setTitle("Window");
    threadIcons = new ThreadIconMap();
    threadInfo = new WindowThreadInfo(masterThread, masterThread->getNumWorkerThreads(), threadIcons);
    addAction("Thread Information", this, SLOT(showInfo()), tr("Ctrl+I"));

    threadStateKey =  new WindowThreadInfoKey(threadIcons);
    addAction("Thread State key", this, SLOT(showKey()));
}

void WindowMenu::showInfo()
{
    threadInfo->raise();
    threadInfo->show();
}

void WindowMenu::showKey()
{
    threadStateKey->raise();
    threadStateKey->show();
}

const ThreadIconMap *WindowMenu::getThreadIcons() const
{
    return threadIcons;
}

QSize WindowMenu::getThreadInfoWindowSize() const
{
    return threadInfo->size();
}

QPoint WindowMenu::getThreadInfoWindowPos() const
{
    return threadInfo->pos();
}
