#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sourcegraphicsscene.h"
#include "computationhandler.h"

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

#define SOURCE_CHECKERBOARD_SIZE 10

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

    // Enable mouse tracking
    ui->graphicsViewSource->setMouseTracking(true);
    ui->graphicsViewTarget->setMouseTracking(true);

    // Enable antialiasing for the graphics views
    ui->graphicsViewSource->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    ui->graphicsViewTarget->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Create a Pixmap item for each graphics scene
    m_pix_item_source = new QGraphicsPixmapItem;
    m_pix_item_target = new QGraphicsPixmapItem;

    // Ensure the pixmap scaling is smooth
    m_pix_item_source->setTransformationMode(Qt::SmoothTransformation);
    m_pix_item_target->setTransformationMode(Qt::SmoothTransformation);

    m_scene_source->addItem(m_pix_item_source);
    m_scene_target->addItem(m_pix_item_target);

    // Create the source checkerboard background
    updateSourceCheckerboard();

    /*
     * Interface actions connection
     */
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->actionOpen_source_image, SIGNAL(triggered(bool)), this, SLOT(openSourceImage()));
    connect(ui->actionOpen_target_image, SIGNAL(triggered(bool)), this, SLOT(openTargetImage()));


    // Temp test action
    connect(ui->actionTemp_Test, SIGNAL(triggered(bool)), this, SLOT(tempTestAction()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pix_item_source;
    delete m_pix_item_target;
}


void MainWindow::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);

    // Fit graphics views to the pixmap items
    ui->graphicsViewSource->fitInView(m_scene_source->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsViewTarget->fitInView(m_scene_target->sceneRect(), Qt::KeepAspectRatio);

    // Update the checkerboard background
    updateSourceCheckerboard();
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

    // Fit graphics view to the pixmap item
    ui->graphicsViewSource->fitInView(m_scene_source->sceneRect(), Qt::KeepAspectRatio);

    // Update the source checkerboard
    updateSourceCheckerboard();
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

void MainWindow::updateSourceCheckerboard() {
    // Define a checkerboard background for the graphics views
    QBrush checkerboard_brush(Qt::lightGray, Qt::Dense4Pattern);

    // We want a checkerboard that is invariant to the graphics view scale.
    // Create a tranformation matrix which is the inverse of the graphics view matrix.
    QTransform transform = ui->graphicsViewSource->transform().inverted();

    // Zoom on the checkerboard to get squares of 20x20
    transform.scale(SOURCE_CHECKERBOARD_SIZE, SOURCE_CHECKERBOARD_SIZE);

    // Apply the resulting transformation matrix to the brush
    checkerboard_brush.setTransform(transform);

    // Apply the checkerboard brush
    ui->graphicsViewSource->setBackgroundBrush(checkerboard_brush);
}

/**
 * @brief MainWindow::tempTestAction
 *
 * This function is a temporary function used for development
 */
void MainWindow::tempTestAction() {
    if (!m_scene_source->isSelectionValid())
        return;

    QImage img_part = m_source_image.copy(m_scene_source->getSelectionPath().boundingRect().toRect());

    qDebug() << "Selection bounding rect:" << m_scene_source->getSelectionPath().boundingRect().toRect();

    QElapsedTimer e_t;
    e_t.start();

    std::array<MatrixXd,3> img_matrices = ComputationHandler::imageToMatrices(img_part);

    qDebug() << "Conversion duration:" << e_t.nsecsElapsed() << "ns";

    qDebug() << "red:" << img_matrices[0](0,0)
             << "\tgreen:" << img_matrices[1](0,0)
             << "\tblue:" << img_matrices[2](0,0);


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

    qDebug() << "Diagonal duration:" << e_t.nsecsElapsed() << "ns";

    qDebug() << mat_test2.nonZeros() << mat_test2.coeff(0,0);
}
