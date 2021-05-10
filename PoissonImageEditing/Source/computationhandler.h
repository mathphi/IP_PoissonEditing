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


class QRunnable;
class QThreadPool;
class PastedSourceItem;
class TransferComputationUnit;
class BlendingComputationUnit;

class ComputationHandler : public QObject
{
    Q_OBJECT

public:
    explicit ComputationHandler(QObject *parent = nullptr);

    void startComputationJob(QRunnable *cu);

    static ImageMatricesRGB imageToMatrices(QImage img);
    static MatrixXd imageToChannelMatrix(QImage img, int channel);
    static QImage matricesToImage(ImageMatricesRGB im_rgb);
    static QImage matricesToImage(ImageMatricesRGB im_rgb, MatrixXd alpha_mask);
    static MatrixXd vectorToMatrixImage(VectorXd img_vect, QSize img_size);

    static SelectMaskMatrices selectionToMask(QPainterPath selection_path);

    static SparseMatrixXd laplacianMatrix(const QSize img_size, SelectMaskMatrices masks);

    static VectorXd computeImageGradient(MatrixXd img_ch, SelectMaskMatrices masks);
    static VectorXd computeImageGradientMixed(MatrixXd img_ch, VectorXd other_grad, SelectMaskMatrices masks);
    static VectorXd computeBoundaryNeighbors(MatrixXd tgt_img_ch, SelectMaskMatrices masks);

private:
    QThreadPool *m_thread_pool;
};

#endif // COMPUTATIONHANDLER_H
