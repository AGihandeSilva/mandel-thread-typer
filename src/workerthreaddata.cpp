#include "workerthreaddata.h"


WorkerThreadData::WorkerThreadData(MandelbrotWidget* owner, uint pass): pass(pass), owner(owner)
{
    connect(this, SIGNAL(writeToLog(QString,bool)), owner, SLOT(outputToLog(QString,bool)));
}

WorkerThreadData::~WorkerThreadData() {
    emit writeToLog("deleting thread data! pass: " + QString::number(pass));
}
