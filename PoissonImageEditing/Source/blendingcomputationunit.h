#ifndef BLENDINGCOMPUTATIONUNIT_H
#define BLENDINGCOMPUTATIONUNIT_H

#include <QObject>
#include <QRunnable>

#include "computationhandler.h"

class BlendingComputationUnit : public QObject, public QRunnable
{
    Q_OBJECT

public:
    BlendingComputationUnit(
            int channel_num,
            QImage target_img,
            VectorXd src_grad,
            SelectMaskMatrices masks,
            SparseMatrixXd laplacian,
            bool mixed_blending
        );

    void run() override;

    int getChannelNumber();
    MatrixXd getBlendedChannel();

signals:
    void computationStarted();
    void computationFinished();

private:
    void computeBlendingData();

    // Input attributes
    int m_channel_num;
    QImage m_target_img;
    VectorXd m_src_grad;
    SelectMaskMatrices m_masks;
    SparseMatrixXd m_laplacian;
    bool m_mixed_blending;

    // Output attributes
    MatrixXd m_blended_channel;
};

#endif // BLENDINGCOMPUTATIONUNIT_H
