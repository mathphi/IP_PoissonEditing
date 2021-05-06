#ifndef COMPUTATIONHANDLER_H
#define COMPUTATIONHANDLER_H

#include <QObject>

#include <Eigen/Core>
#include <Eigen/Sparse>


/*
 * Eigen matrices type definitions
 */
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
typedef Eigen::SparseMatrix<float> SparseMatrixXd;


class ComputationHandler : public QObject
{
    Q_OBJECT
public:
    explicit ComputationHandler(QObject *parent = nullptr);

    static std::array<MatrixXd,3> imageToMatrices(QImage img);

signals:

};

#endif // COMPUTATIONHANDLER_H
