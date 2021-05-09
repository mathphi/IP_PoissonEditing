#ifndef COMPUTATIONHANDLER_H
#define COMPUTATIONHANDLER_H

#include <QImage>
#include <QObject>
#include <QPainterPath>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>


/*
 * Eigen matrices type definitions
 */
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
typedef Eigen::SparseMatrix<float> SparseMatrixXd;
typedef Eigen::Matrix<float, Eigen::Dynamic, 1> VectorXd;

typedef std::array<MatrixXd,3> ImageMatricesRGB;
typedef std::array<VectorXd,3> ImageVectorRGB;

struct SelectMaskMatrices {
    MatrixXd positive_mask;
    MatrixXd negative_mask;
};


class QThreadPool;
class PastedSourceItem;
class TransferComputationUnit;

class ComputationHandler : public QObject
{
    Q_OBJECT

public:
    explicit ComputationHandler(QObject *parent = nullptr);

    TransferComputationUnit *startSourceTransferJob(PastedSourceItem *origin);


    static ImageMatricesRGB imageToMatrices(QImage img);
    static QImage matricesToImage(ImageMatricesRGB im_rgb);
    static QImage matricesToImage(ImageMatricesRGB im_rgb, MatrixXd alpha_mask);
    static SelectMaskMatrices selectionToMask(QPainterPath selection_path);

    static SparseMatrixXd laplacianMatrix(const QSize img_size, SelectMaskMatrices masks);

    static VectorXd computeImageGradient(MatrixXd img_ch, SelectMaskMatrices masks);

    static VectorXd computeBoundaryNeighbors(MatrixXd tgt_img_ch, SelectMaskMatrices masks);

    static MatrixXd vectorToMatrixImage(VectorXd img_vect, QSize img_size);

signals:

private:
    QThreadPool *m_thread_pool;
};

#endif // COMPUTATIONHANDLER_H
