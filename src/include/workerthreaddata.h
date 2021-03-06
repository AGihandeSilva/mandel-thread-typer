#ifndef WORKERTHREADDATA_H
#define WORKERTHREADDATA_H

#include <QString>

#include <vector>
#include "mandelbrotwidget.h"

class WorkerThreadData : public QObject
{
    Q_OBJECT

public:
    WorkerThreadData(MandelbrotWidget* owner = nullptr, uint pass = 0);

    ~WorkerThreadData() override;
    WorkerThreadData(const WorkerThreadData&) = delete;
    WorkerThreadData(WorkerThreadData&&) = delete;
    WorkerThreadData& operator= (const WorkerThreadData&) = delete;
    WorkerThreadData& operator= (WorkerThreadData&&) = delete;

    uint getPassData() const { return pass; }
    std::vector<int>& getValues() { return values; }

signals:
    void writeToLog(const QString& outputString, bool show = false) const;

private:
    const uint pass;
    std::vector<int> values;
    MandelbrotWidget* owner;
};

#endif // WORKERTHREADDATA_H
