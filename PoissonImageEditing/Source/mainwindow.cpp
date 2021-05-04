#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sourcegraphicsscene.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>

#include <QDebug>

#define IMAGE_EXTENSIONS "PNG (*.png);;JPG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tif *.tiff)"


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

    m_scene_source->addItem(m_pix_item_source);
    m_scene_target->addItem(m_pix_item_target);

    // Define a checkerboard background for the graphics views
    QBrush checkerboard_brush(Qt::lightGray, Qt::Dense4Pattern);

    // Zoom on the checkerboard to get squares of 20x20
    QTransform transform = checkerboard_brush.transform();
    transform.scale(20, 20);
    checkerboard_brush.setTransform(transform);

    // Apply the checkerboard brush
    ui->graphicsViewSource->setBackgroundBrush(checkerboard_brush);


    /*
     * Interface actions connection
     */
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->actionOpen_source_image, SIGNAL(triggered(bool)), this, SLOT(openSourceImage()));
    connect(ui->actionOpen_target_image, SIGNAL(triggered(bool)), this, SLOT(openTargetImage()));
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
}


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

void MainWindow::updateSourceScene() {
    // Update the picture of the pixmap item
    m_pix_item_source->setPixmap(QPixmap::fromImage(m_source_image));

    // Set the scene rect to the source image rect
    m_scene_source->setSceneRect(0, 0, m_source_image.width(), m_source_image.height());

    // Fit graphics view to the pixmap item
    ui->graphicsViewSource->fitInView(m_scene_source->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::updateTargetScene() {
    // Update the picture of the pixmap item
    m_pix_item_target->setPixmap(QPixmap::fromImage(m_target_image));

    // Set the scene rect to the target image rect
    m_scene_target->setSceneRect(0, 0, m_target_image.width(), m_target_image.height());

    // Fit graphics view to the pixmap item
    ui->graphicsViewTarget->fitInView(m_scene_target->sceneRect(), Qt::KeepAspectRatio);
}
