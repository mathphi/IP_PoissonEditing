#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sourcegraphicsscene.h"
#include "computationhandler.h"
#include "pastedsourceitem.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QElapsedTimer>

#include <QThread>
#include <QDebug>

#define IMAGE_EXTENSIONS "All Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;" \
                         "PNG (*.png);;JPG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tif *.tiff)"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create graphics scenes
    m_scene_source = new SourceGraphicsScene(this);
    m_scene_target = new QGraphicsScene(this);

    // Bind graphics scenes to their view
    ui->graphicsViewSource->setScene(m_scene_source);
    ui->graphicsViewTarget->setScene(m_scene_target);

    // Create a Pixmap item for each graphics scene
    m_pix_item_source = new QGraphicsPixmapItem;
    m_pix_item_target = new QGraphicsPixmapItem;

    // Ensure the pixmap scaling is smooth
    m_pix_item_source->setTransformationMode(Qt::SmoothTransformation);
    m_pix_item_target->setTransformationMode(Qt::SmoothTransformation);

    m_scene_source->addItem(m_pix_item_source);
    m_scene_target->addItem(m_pix_item_target);

    // Initialise UI component state
    ui->transferButton->setEnabled(false);

    /*
     * Interface actions connection
     */
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->actionOpen_source_image, SIGNAL(triggered(bool)), this, SLOT(openSourceImage()));
    connect(ui->actionOpen_target_image, SIGNAL(triggered(bool)), this, SLOT(openTargetImage()));

    connect(m_scene_source, SIGNAL(lassoDrawn(QPainterPath)), this, SLOT(sourceLassoDrawn(QPainterPath)));
    connect(m_scene_source, SIGNAL(lassoRemoved()), this, SLOT(sourceLassoRemoved()));

    // Temp test action
    connect(ui->actionTemp_Test, SIGNAL(triggered(bool)), this, SLOT(tempTestAction()));
    connect(ui->transferButton, SIGNAL(clicked()), this, SLOT(tempTestAction()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pix_item_source;
    delete m_pix_item_target;
}


/**
 * @brief MainWindow::openSourceImage
 *
 * This slot is called when the "Open source image" action is triggered
 */
void MainWindow::openSourceImage() {
    // Open existing image file
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open a source image",
                QDir::homePath(),
                IMAGE_EXTENSIONS);

    // If dialog was closed
    if (filename.isEmpty())
        return;

    m_source_image = QImage(filename);

    // Update the scene according to the newly opened image
    updateSourceScene();

    // Remove the current lasso (if any)
    m_scene_source->removeLasso();

    // The lasso can be enabled since we have opened an image
    m_scene_source->enableLasso(true);
}

/**
 * @brief MainWindow::openTargetImage
 *
 * This slot is called when the "Open target image" action is triggered
 */
void MainWindow::openTargetImage() {
    // Open existing image file
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open a target image",
                QDir::homePath(),
                IMAGE_EXTENSIONS);

    // If dialog was closed
    if (filename.isEmpty())
        return;

    m_target_image = QImage(filename);

    updateTargetScene();

    // Enable transfer button if a lasso is already drawn
    if (m_scene_source->isSelectionValid()) {
        ui->transferButton->setEnabled(true);
    }
}

/**
 * @brief MainWindow::updateSourceScene
 *
 * This slot updates the pixmap on the source graphics scene
 */
void MainWindow::updateSourceScene() {
    // Update the picture of the pixmap item
    m_pix_item_source->setPixmap(QPixmap::fromImage(m_source_image));

    // Set the scene rect to the source image rect
    m_scene_source->setSceneRect(0, 0, m_source_image.width(), m_source_image.height());
}

/**
 * @brief MainWindow::updateTargetScene
 *
 * This slot updates the pixmap on the target graphics scene
 */
void MainWindow::updateTargetScene() {
    // Update the picture of the pixmap item
    m_pix_item_target->setPixmap(QPixmap::fromImage(m_target_image));

    // Set the scene rect to the target image rect
    m_scene_target->setSceneRect(0, 0, m_target_image.width(), m_target_image.height());

    // Fit graphics view to the pixmap item
    ui->graphicsViewTarget->fitInView(m_scene_target->sceneRect(), Qt::KeepAspectRatio);
}

/**
 * @brief MainWindow::sourceLassoDrawn
 * @param path
 *
 * This slot is called when a lasso has been drawn in the source scene
 */
void MainWindow::sourceLassoDrawn(QPainterPath path) {
    Q_UNUSED(path);

    if (!m_target_image.isNull()) {
        ui->transferButton->setEnabled(true);
    }
}

/**
 * @brief MainWindow::sourceLassoRemoved
 *
 * This slot is called when a lasso has been removed from the source scene
 */
void MainWindow::sourceLassoRemoved() {
    ui->transferButton->setEnabled(false);
}

/**
 * @brief MainWindow::tempTestAction
 *
 * This function is a temporary function used for development
 */
void MainWindow::tempTestAction() {
    if (!m_scene_source->isSelectionValid())
        return;

    QRect select_rect = m_scene_source->getSelectionPath().boundingRect().toAlignedRect();
    select_rect.adjust(-1, -1, 1, 1);

    QImage img_part = m_source_image.copy(select_rect);

    qDebug() << "Selection bounding rect:" << select_rect;

    QElapsedTimer e_t;
    e_t.start();

    ImageMatricesRGB img_matrices = ComputationHandler::imageToMatrices(img_part);

    qDebug() << "Conversion duration:" << e_t.nsecsElapsed() << "ns";

    qDebug() << "red:" << img_matrices[0](0,0)
             << "\tgreen:" << img_matrices[1](0,0)
             << "\tblue:" << img_matrices[2](0,0);

/*
    e_t.restart();

    SparseMatrixXd mat_test(1000000,1000000);

    qDebug() << "Allocation duration:" << e_t.nsecsElapsed() << "ns";
    e_t.restart();

    mat_test.setIdentity();

    qDebug() << "Identity duration:" << e_t.nsecsElapsed() << "ns";
    e_t.restart();

    mat_test = mat_test * 4;

    qDebug() << "Scalar multiply duration:" << e_t.nsecsElapsed() << "ns";
    e_t.restart();

    float coeff = mat_test.coeff(0,0);

    qDebug() << "Lookup duration:" << e_t.nsecsElapsed() << "ns";

    qDebug() << mat_test.rows() << mat_test.cols() << coeff;


    SparseMatrixXd mat_test2(1000000,1000000);

    e_t.restart();

    mat_test2.reserve(Eigen::VectorXi::Constant(mat_test.cols(),5));
    for (int i = 0 ; i < mat_test.cols() ; i++) {
        mat_test2.insert(i,i) = 4.0;

        if (i > 0) {
            mat_test2.insert(i,i-1) = -1.0;
        }
        if (i < 1000000-1) {
            mat_test2.insert(i,i+1) = -1.0;
        }
        if (i > 1000-1) {
            mat_test2.insert(i,i-1000) = -1.0;
        }
        if (i < 1000000-1000-1) {
            mat_test2.insert(i,i+1000) = -1.0;
        }
    }

    qDebug() << "Insertion duration:" << e_t.nsecsElapsed() << "ns";

    qDebug() << mat_test2.nonZeros() << mat_test2.coeff(0,0);

*/
    e_t.restart();

    SelectMaskMatrices smm = ComputationHandler::selectionToMask(m_scene_source->getSelectionPath());

    qDebug() << "Mask generation duration:" << e_t.nsecsElapsed() << "ns";
    e_t.restart();

    ImageMatricesRGB result;
    result[0] = img_matrices[0].cwiseProduct(smm.positive_mask);
    result[1] = img_matrices[1].cwiseProduct(smm.positive_mask);
    result[2] = img_matrices[2].cwiseProduct(smm.positive_mask);

    qDebug() << "Elementwise product duration:" << e_t.nsecsElapsed() << "ns";

    QImage img = ComputationHandler::matricesToImage(result, smm.positive_mask);



    // Prepare the Source Image Pack for the item
    SourceImagePack img_pack;
    img_pack.image = img;
    img_pack.matrices = result;
    img_pack.masks = smm;

    // Normalize the selection path to its bounding rect (with 1px margin)
    QPainterPath p = m_scene_source->getSelectionPath();
    p.translate(-p.boundingRect().topLeft() + QPointF(1,1));

    // Create the Pasted Source Item
    PastedSourceItem *src_item = new PastedSourceItem(img_pack, p);
    m_scene_target->addItem(src_item);

    // Place the item on the center of the target
    src_item->setPos(QPointF(m_scene_target->sceneRect().width()/2 - src_item->boundingRect().width()/2,
                             m_scene_target->sceneRect().height()/2 - src_item->boundingRect().height()/2));

    // Set the new item as selected
    src_item->setSelected(true);



    e_t.restart();

    SparseMatrixXd laplacian_mat = ComputationHandler::laplacianMatrix(img_part.size());

    qDebug() << "Laplacian generation duration:" << e_t.nsecsElapsed() << "ns";
    qDebug() << "Size:" << laplacian_mat.rows() << "x" << laplacian_mat.cols();
    qDebug() << "NNZ:" << laplacian_mat.nonZeros();


}
