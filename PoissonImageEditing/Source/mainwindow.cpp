#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sourcegraphicsscene.h"
#include "targetgraphicsscene.h"
#include "computationhandler.h"
#include "pastedsourceitem.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QImageReader>
#include <QImageWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>

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
    m_scene_target = new TargetGraphicsScene(this);

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

    /*
     * Signal/slot connections
     */

    //Interface actions connection
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->actionOpen_source_image, SIGNAL(triggered(bool)), this, SLOT(openSourceImage()));
    connect(ui->actionOpen_target_image, SIGNAL(triggered(bool)), this, SLOT(openTargetImage()));

    connect(ui->actionDelete_selected_layer, SIGNAL(triggered(bool)), m_scene_target, SLOT(removeSelectedSrcItem()));
    connect(ui->actionDelete_all_layers,     SIGNAL(triggered(bool)), this,           SLOT(askRemoveAllLayers()));

    connect(ui->actionTransfer_selection, SIGNAL(triggered(bool)), this, SLOT(transferLassoSelection()));
    connect(ui->transferButton,           SIGNAL(clicked()),       this, SLOT(transferLassoSelection()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered(bool)), this, SLOT(aboutQtDialog()));
    connect(ui->actionAbout,    SIGNAL(triggered(bool)), this, SLOT(aboutProgramDialog()));

    // Source scene signals
    connect(m_scene_source, SIGNAL(lassoDrawn(QPainterPath)), this, SLOT(sourceLassoDrawn(QPainterPath)));
    connect(m_scene_source, SIGNAL(lassoRemoved()),           this, SLOT(sourceLassoRemoved()));

    // Target scene signals
    connect(m_scene_target, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(targetSceneKeyPressed(QKeyEvent*)));
    connect(m_scene_target, SIGNAL(selectionChanged()),     this, SLOT(targetSceneSelectionChanged()));

    // Drag & drop actions from graphics views
    connect(ui->graphicsViewSource, SIGNAL(imageFileDropped(QString)), this, SLOT(openSourceImage(QString)));
    connect(ui->graphicsViewTarget, SIGNAL(imageFileDropped(QString)), this, SLOT(openTargetImage(QString)));

    // Temp test action
    connect(ui->actionTemp_Test, SIGNAL(triggered(bool)), this, SLOT(tempTestAction()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pix_item_source;
    delete m_pix_item_target;
}

/**
 * @brief MainWindow::aboutQtDialog
 *
 * This slot opens the "About Qt" dialog
 */
void MainWindow::aboutQtDialog() {
    QMessageBox::aboutQt(this);
}

/**
 * @brief MainWindow::aboutProgramDialog
 *
 * This slot opens the "About" dialog of the program
 */
void MainWindow::aboutProgramDialog() {
    // Get the about content from a resource file
    QFile about_file(":/texts/about.html");

    if (!about_file.open(QIODevice::ReadOnly))
        return;

    QString about_content = about_file.readAll();

    QMessageBox::about(
                this,
                "About Poisson Image Blending",
                about_content);
}

/**
 * @brief MainWindow::openSourceImage
 *
 * This slot is called when the "Open source image" action is triggered.
 */
void MainWindow::openSourceImage(QString filename) {
    // If no filename was specified -> file dialog
    if (filename.isEmpty()) {
        // Open existing image file
        filename = QFileDialog::getOpenFileName(
                        this,
                        "Open a source image",
                        QDir::homePath(),
                        IMAGE_EXTENSIONS);
    }

    // If dialog was closed
    if (filename.isEmpty())
        return;

    // Open the image file and get its content
    QImage new_image(filename);

    // Check if this is a valid image
    if (new_image.isNull()) {
        QMessageBox::critical(
                    this,
                    "File error",
                    "The specified image file could not be opened.");
    }

    // Update the source image
    m_source_image = new_image;

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
void MainWindow::openTargetImage(QString filename) {
    // If no filename was specified -> file dialog
    if (filename.isEmpty()) {
        // Open existing image file
        filename = QFileDialog::getOpenFileName(
                        this,
                        "Open a target image",
                        QDir::homePath(),
                        IMAGE_EXTENSIONS);
    }

    // If dialog was closed
    if (filename.isEmpty())
        return;

    // Open the image file and get its content
    QImage new_image(filename);

    // Check if this is a valid image
    if (new_image.isNull()) {
        QMessageBox::critical(
                    this,
                    "File error",
                    "The specified image file could not be opened.");
    }

    // Update the target image
    m_target_image = new_image;

    // Update the scene according to the newly opened image
    updateTargetScene();

    // Enable transfer button if a lasso is already drawn
    if (m_scene_source->isSelectionValid()) {
        ui->transferButton->setEnabled(true);
        ui->actionTransfer_selection->setEnabled(true);
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
        ui->actionTransfer_selection->setEnabled(true);
    }
}

/**
 * @brief MainWindow::sourceLassoRemoved
 *
 * This slot is called when a lasso has been removed from the source scene
 */
void MainWindow::sourceLassoRemoved() {
    ui->transferButton->setEnabled(false);
    ui->actionTransfer_selection->setEnabled(false);
}

/**
 * @brief MainWindow::targetSceneKeyPressed
 * @param event
 *
 * This slot is called when the target scene receives a key event.
 */
void MainWindow::targetSceneKeyPressed(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Escape: {
        // Escape -> deselect all items
        m_scene_target->clearSelection();
        m_scene_target->clearFocus();
        break;
    }
    case Qt::Key_Delete: {
        if (event->modifiers() & Qt::ControlModifier) {
            // Delete with control key
            // -> remove all the pasted items (ask before)
            askRemoveAllLayers();
        }
        else {
            // Delete without control key
            // -> remove the selected item (if one)
            m_scene_target->removeSelectedSrcItem();
        }
        break;
    }
    case Qt::Key_F5: {
        //TODO: recompute selected/all pasted sources (?)
        break;
    }
    default:
        break;
    }
}

/**
 * @brief MainWindow::targetSceneSelectionChanged
 *
 * This slot is called by the scene when the item selection changed.
 */
void MainWindow::targetSceneSelectionChanged() {
    // Get the number of selected items
    int selection_count = m_scene_target->selectedItems().size();

    // These actions are enabled only if an item is selected
    ui->actionDelete_selected_layer->setEnabled(selection_count > 0);
    ui->actionRecompute_selected_layer->setEnabled(selection_count > 0);
}

/**
 * @brief MainWindow::askRemoveAllLayers
 *
 * This slot asks before removing all pasted layers from the target scene.
 */
void MainWindow::askRemoveAllLayers() {
    int ans = QMessageBox::warning(
                this,
                "Delete confirmation",
                "This will delete the all the pasted items.\nAre you sure you want to continue ?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

    // Remove all if the answer is yes
    if (ans == QMessageBox::Yes) {
        m_scene_target->removeAllSrcItem();
    }
}

/**
 * @brief MainWindow::transferLassoSelection
 *
 * This slot transfers the selection from the source to the target scene.
 */
void MainWindow::transferLassoSelection() {
    // Abort if there is no valid selection
    if (!m_scene_source->isSelectionValid())
        return;

    // Abort if the target is not ready
    if (m_target_image.isNull())
        return;


    tempTestAction();

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


    e_t.restart();

    SparseMatrixXd laplacian_mat = ComputationHandler::laplacianMatrix(img_part.size());

    qDebug() << "Laplacian generation duration:" << e_t.nsecsElapsed() << "ns";
    qDebug() << "Size:" << laplacian_mat.rows() << "x" << laplacian_mat.cols();
    qDebug() << "NNZ:" << laplacian_mat.nonZeros();


    if (!m_scene_target->isRectangleInsertable(img.rect())) {
        QMessageBox::critical(
                    NULL,
                    "Oversized source",
                    "The source item you are trying to paste is larger than the target.\n"
                    "Try again with a smaller source or a larger target.");
    }
    else {
        // Prepare the Source Image Pack for the item
        SourceImagePack img_pack;
        img_pack.image = img;
        img_pack.matrices = result;
        img_pack.masks = smm;

        // Normalize the selection path to its bounding rect (with 1px margin)
        QPainterPath p = m_scene_source->getSelectionPath();
        p.translate(-p.boundingRect().topLeft()
                    + QPointF((img.width() - p.boundingRect().width()) / 2.0, (img.height() - p.boundingRect().height()) / 2.0)
                    + QPointF(0.5,0.5));
        // pixel alignment margin + 0.5 to align the selection path on the center of the pixel

        // Create the Pasted Source Item
        PastedSourceItem *src_item = new PastedSourceItem(img_pack, laplacian_mat, p);

        m_scene_target->addSourceItem(src_item);
    }
}
