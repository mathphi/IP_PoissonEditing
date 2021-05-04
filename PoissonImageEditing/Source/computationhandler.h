#ifndef COMPUTATIONHANDLER_H
#define COMPUTATIONHANDLER_H

#include <QObject>

#include <Eigen/Core>


/*
 * Eigen matrices type definitions
 */
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;


class ComputationHandler : public QObject
{
    Q_OBJECT
public:
    explicit ComputationHandler(QObject *parent = nullptr);

    static std::array<MatrixXd,3> imageToMatrices(QImage img);

signals:

};

#endif // COMPUTATIONHANDLER_H
