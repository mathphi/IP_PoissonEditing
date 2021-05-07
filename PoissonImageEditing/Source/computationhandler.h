#ifndef COMPUTATIONHANDLER_H
#define COMPUTATIONHANDLER_H

#include <QObject>
#include <QPainterPath>

#include <Eigen/Core>
#include <Eigen/Sparse>


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


class ComputationHandler : public QObject
{
    Q_OBJECT

public:
    explicit ComputationHandler(QObject *parent = nullptr);

    static ImageMatricesRGB imageToMatrices(QImage img);
    static QImage matricesToImage(ImageMatricesRGB im_rgb);
    static QImage matricesToImage(ImageMatricesRGB im_rgb, MatrixXd alpha_mask);
    static SelectMaskMatrices selectionToMask(QPainterPath selection_path);

    static SparseMatrixXd laplacianMatrix(const QSize img_size);

    static VectorXd computeBoundaryConditionsSimple(MatrixXd src_img_ch, MatrixXd tgt_img_ch, SelectMaskMatrices masks);

signals:

};

#endif // COMPUTATIONHANDLER_H
