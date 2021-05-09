#ifndef TRANSFERCOMPUTATIONUNIT_H
#define TRANSFERCOMPUTATIONUNIT_H

#include <QRunnable>
#include <QImage>
#include <QPainterPath>

class PastedSourceItem;

class TransferComputationUnit : public QRunnable
{
public:
    TransferComputationUnit(PastedSourceItem *origin);

    void run() override;

private:
    void computeTransferData();

    PastedSourceItem *m_origin;
};

#endif // TRANSFERCOMPUTATIONUNIT_H
