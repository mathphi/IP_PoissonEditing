#ifndef COMPUTATIONHANDLER_H
#define COMPUTATIONHANDLER_H

#include <QImage>
#include <QObject>
#include <QPainterPath>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

#include "array"


/*
 * Eigen matrices type definitions
 */
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
typedef Eigen::SparseMatrix<float> SparseMatrixXd;
typedef Eigen::Matrix<float, Eigen::Dynamic, 1> VectorXd;

typedef std::array<MatrixXd,3> ImageMatricesRGB;

struct SelectMaskMatrices {
    MatrixXd positive_mask;
    MatrixXd negative_mask;
};


class QRunnable;
class QThreadPool;
class PastedSourceItem;
class TransferComputationUnit;
class BlendingComputationUnit;

class ComputationHandler
{
public:
    static void initializeComputationHandler(QObject *parent = nullptr);
    static bool startComputationJob(QRunnable *cu);

    static ImageMatricesRGB imageToMatrices(QImage img);
    static MatrixXd imageToChannelMatrix(QImage img, int channel);
    static QImage matricesToImage(ImageMatricesRGB im_rgb);
    static QImage matricesToImage(ImageMatricesRGB im_rgb, MatrixXd alpha_mask);
    static MatrixXd vectorToMatrixImage(VectorXd img_vect, QSize img_size);

    static SelectMaskMatrices selectionToMask(QPainterPath selection_path);

    static SparseMatrixXd laplacianMatrix(const QSize img_size, SelectMaskMatrices masks);

    static VectorXd computeImageGradient(MatrixXd img_ch, SelectMaskMatrices masks);
    static VectorXd computeImagesGradientMixed(MatrixXd img1_ch, MatrixXd img2_ch, SelectMaskMatrices masks);
    static VectorXd computeBoundaryNeighbors(MatrixXd tgt_img_ch, SelectMaskMatrices masks);
};


/*
 * Types serialization functions definition
 */

// Standard array serialization
template <typename T, std::size_t N>
inline QDataStream &operator>>(QDataStream &in, std::array<T, N> &p) {
    for (std::size_t i = 0 ; i < p.size() ; i++) {
        in >> p[i];
    }
    return in;
}

template <typename T, std::size_t N>
inline QDataStream &operator<<(QDataStream &out, std::array<T, N> &p) {
    for (std::size_t i = 0 ; i < p.size() ; i++) {
        out << p[i];
    }
    return out;
}

// Eigen matrix serialization
QDataStream &operator>>(QDataStream &in, MatrixXd &p);
QDataStream &operator<<(QDataStream &out, MatrixXd &p);

// Eigen vector serialization
QDataStream &operator>>(QDataStream &in, VectorXd &p);
QDataStream &operator<<(QDataStream &out, VectorXd &p);

// Eigen sparse matrix serialization
QDataStream &operator>>(QDataStream &in, SparseMatrixXd &p);
QDataStream &operator<<(QDataStream &out, SparseMatrixXd &p);

// SelectMaskMatrices serialization
QDataStream &operator>>(QDataStream &in, SelectMaskMatrices &p);
QDataStream &operator<<(QDataStream &out, SelectMaskMatrices &p);


#endif // COMPUTATIONHANDLER_H
