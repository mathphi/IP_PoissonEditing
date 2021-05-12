#ifndef TRANSFERCOMPUTATIONUNIT_H
#define TRANSFERCOMPUTATIONUNIT_H

#include <QRunnable>
#include <QImage>
#include <QPainterPath>

#include "computationhandler.h"

class PastedSourceItem;

class TransferComputationUnit : public QObject, public QRunnable
{
    Q_OBJECT

public:
    TransferComputationUnit(QImage source_image, QPainterPath selection_path);

    void run() override;

    ImageMatricesRGB getOriginalMatrices();
    SelectMaskMatrices getMasks();
    QImage getOriginalImageMasked();
    SparseMatrixXd getLaplacian();

signals:
    void computationStarted();
    void computationFinished();

private:
    void computeTransferData();

    // Input attributes
    QImage m_source_image;
    QPainterPath m_selection_path;

    // Output attributes
    ImageMatricesRGB m_original_matrices;
    SelectMaskMatrices m_masks;
    QImage m_original_image_masked;
    SparseMatrixXd m_laplacian;
};

#endif // TRANSFERCOMPUTATIONUNIT_H
