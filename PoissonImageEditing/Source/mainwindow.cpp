#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    m_scene_source = new QGraphicsScene(this);
    m_scene_target = new QGraphicsScene(this);

    ui->graphicsViewSource->setScene(m_scene_source);
    ui->graphicsViewTarget->setScene(m_scene_target);

    m_pix_item_source = new QGraphicsPixmapItem;
    m_pix_item_target = new QGraphicsPixmapItem;

    m_scene_source->addItem(m_pix_item_source);
    m_scene_target->addItem(m_pix_item_target);

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
    ui->graphicsViewSource->fitInView(m_pix_item_source, Qt::KeepAspectRatio);
    ui->graphicsViewTarget->fitInView(m_pix_item_target, Qt::KeepAspectRatio);
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

    updateSourceScene();
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

    // Fit graphics view to the pixmap item
    ui->graphicsViewSource->fitInView(m_pix_item_source, Qt::KeepAspectRatio);
}

void MainWindow::updateTargetScene() {
    // Update the picture of the pixmap item
    m_pix_item_target->setPixmap(QPixmap::fromImage(m_target_image));

    // Fit graphics view to the pixmap item
    ui->graphicsViewTarget->fitInView(m_pix_item_target, Qt::KeepAspectRatio);
}
