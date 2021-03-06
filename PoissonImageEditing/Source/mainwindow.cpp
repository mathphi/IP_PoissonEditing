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

#define PROGRAM_SIGNATURE   "PIB-ELECY412"
#define PROJECT_FILE_EXT    "Poisson Image Blending Project (*.pibproj)"
#define IMAGE_EXTENSIONS    "All Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.gif);;" \
                            "PNG (*.png);;JPG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tif *.tiff);;GIF (*.gif)"
#define IMAGE_WRITE_EXT     "PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;TIFF (*.tif);;GIF (*.gif)"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create status bar and add it to MainWindow
    m_status_bar = new QStatusBar(this);
    this->setStatusBar(m_status_bar);

    // Create a label to display selection rect size
    m_label_size = new QLabel();
    m_status_bar->addPermanentWidget(m_label_size);

    // Initialize the computation handler
    ComputationHandler::initializeComputationHandler(this);

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

    connect(ui->actionOpen_project, SIGNAL(triggered(bool)), this, SLOT(openProject()));
    connect(ui->actionSave_project, SIGNAL(triggered(bool)), this, SLOT(saveProject()));

    connect(ui->actionExport,       SIGNAL(triggered(bool)), this, SLOT(exportResultDirect()));
    connect(ui->actionExport_as,    SIGNAL(triggered(bool)), this, SLOT(exportResultAs()));

    connect(ui->actionClear_selection,    SIGNAL(triggered(bool)), this, SLOT(clearLassoSelection()));
    connect(ui->actionTransfer_selection, SIGNAL(triggered(bool)), this, SLOT(transferLassoSelection()));
    connect(ui->transferButton,           SIGNAL(clicked()),       this, SLOT(transferLassoSelection()));

    connect(ui->actionDelete_selected_layer, SIGNAL(triggered(bool)), m_scene_target, SLOT(removeSelectedSrcItem()));
    connect(ui->actionDelete_all_layers,     SIGNAL(triggered(bool)), this,           SLOT(askRemoveAllLayers()));

    connect(ui->actionReal_time_blending, SIGNAL(toggled(bool)), m_scene_target, SLOT(changeRealTimeBlending(bool)));
    connect(ui->actionMixed_blending,     SIGNAL(toggled(bool)), m_scene_target, SLOT(changeMixedBlending(bool)));

    connect(ui->actionRecompute_selected_layer, SIGNAL(triggered(bool)), m_scene_target, SLOT(recomputeBlendingSelected()));
    connect(ui->actionRecompute_all_layers,     SIGNAL(triggered(bool)), m_scene_target, SLOT(recomputeBlendingAll()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered(bool)), this, SLOT(aboutQtDialog()));
    connect(ui->actionAbout,    SIGNAL(triggered(bool)), this, SLOT(aboutProgramDialog()));

    // Source scene signals
    connect(m_scene_source, SIGNAL(lassoDrawn(QPainterPath)), this, SLOT(sourceLassoDrawn(QPainterPath)));
    connect(m_scene_source, SIGNAL(lassoRemoved()),           this, SLOT(sourceLassoRemoved()));

    // Target scene signals
    connect(m_scene_target, SIGNAL(keyPressed(QKeyEvent*)),  this, SLOT(targetSceneKeyPressed(QKeyEvent*)));
    connect(m_scene_target, SIGNAL(selectionChanged()),      this, SLOT(targetSceneSelectionChanged()));
    connect(m_scene_target, SIGNAL(sourceItemListChanged()), this, SLOT(pastedItemListChanged()));

    // Drag & drop actions from graphics views
    connect(ui->graphicsViewSource, SIGNAL(imageFileDropped(QString)), this, SLOT(openSourceImage(QString)));
    connect(ui->graphicsViewTarget, SIGNAL(imageFileDropped(QString)), this, SLOT(openTargetImage(QString)));
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

    // Update UI
    updateUiComponents();
}

/**
 * @brief MainWindow::openTargetImage
 *
 * This slot is called when the "Open target image" action is triggered
 */
void MainWindow::openTargetImage(QString filename) {
    // First, check if there are pasted layers
    if (m_scene_target->getSourceItemList().size() > 0) {
        // If this is the case, ask the user
        int ans = QMessageBox::warning(
                    this,
                    "Target image replacement",
                    "Replacing the target image will delete the currently pasted layers.\n"
                    "Are you sure you want to continue?",
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);

        // If the answer is not yes -> abort
        if (ans != QMessageBox::Yes)
            return;

        // If the answer is yes -> replace the target image
    }

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

    // Remove the currently pasted layers (if one)
    m_scene_target->removeAllSrcItem();

    // Update the scene according to the newly opened image
    updateTargetScene();

    // Update UI
    updateUiComponents();
}

/**
 * @brief MainWindow::openProject
 *
 * This slot opens a file dialog to select a project file from
 * which extract the project's data.
 */
void MainWindow::openProject() {
    // Open existing image file
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open a source image",
                QDir::homePath(),
                PROJECT_FILE_EXT);

    // If dialog was closed
    if (filename.isEmpty())
        return;

    // Check if file exists and is readable
    if (!QFile::exists(filename)) {
        QMessageBox::critical(
                    this,
                    "File opening error",
                    "The selected file does not exist");
        return;
    }

    // Extract the project's data
    openProjectDataFile(filename);

    // Update some UI state
    updateUiComponents();
}

/**
 * @brief MainWindow::saveProject
 *
 * This slot opens a save file dialog and write the project's data
 * into the selected file.
 */
void MainWindow::saveProject() {
    // Ask for export location
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save the project as...",
                QDir::homePath(),
                PROJECT_FILE_EXT);

    // If the file dialog was canceled
    if (filename.isEmpty())
        return;

    // Write project's data into the selected file
    saveProjectDataToFile(filename);
}

/**
 * @brief MainWindow::openProjectDataFile
 * @param filename
 *
 * This function extracts the project's data from the given file.
 */
void MainWindow::openProjectDataFile(QString filename) {
    // Handle the file name
    QFile in_f(filename);

    // Open the file in read only mode
    if (!in_f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
                    this,
                    "Project file opening error",
                    "Unable to open the project file");
        return;
    }

    // Stream the data file
    QDataStream in(&in_f);

    // ----- Signature ----- //
    QString signature;
    in >> signature;

    if (signature != PROGRAM_SIGNATURE) {
        QMessageBox::critical(
                    this,
                    "Project file opening error",
                    "The file you are trying to open appears not to be a compatible project file.");
        return;
    }

    // ----- Base images ----- //
    // Source and target images
    in >> m_source_image;
    in >> m_target_image;

    // Update the source and target with the new images
    updateSourceScene();
    updateTargetScene();

    // Remove the current lasso (if one)
    m_scene_source->removeLasso();

    // ----- Pasted items ----- //
    // New list of pasted source items
    QList<PastedSourceItem*> psi_lst;
    in >> psi_lst;

    // Remove all current pasted source item
    m_scene_target->removeAllSrcItem();

    // Add the new items to the scene
    foreach (PastedSourceItem *item, psi_lst) {
        m_scene_target->addSourceItem(item, false, false);
    }

    // ----- Blending settings ----- //
    bool is_mixed, is_realtime;
    in >> is_mixed;
    in >> is_realtime;

    ui->actionMixed_blending->setChecked(is_mixed);
    ui->actionReal_time_blending->setChecked(is_realtime);

    m_scene_target->changeMixedBlending(is_mixed);
    m_scene_target->changeRealTimeBlending(is_realtime);

    // Recovering from file done !
}

/**
 * @brief MainWindow::saveProjectDataToFile
 * @param filename
 *
 * This function writes the project's data into the given file.
 */
void MainWindow::saveProjectDataToFile(QString filename) {
    // Handle the file name
    QFile out_f(filename);

    // Open the file in write only mode
    if (!out_f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(
                    this,
                    "Project file opening error",
                    "Unable to open the destination project file");
        return;
    }

    // Stream the data file
    QDataStream out(&out_f);

    // ----- Signature ----- //
    QString signature = PROGRAM_SIGNATURE;
    out << signature;

    // ----- Base images ----- //
    // Source and target images
    out << m_source_image;
    out << m_target_image;

    // ----- Pasted items ----- //
    // Store the list of pasted source items
    out << m_scene_target->getSourceItemList();

    // ----- Blending settings ----- //
    out << ui->actionMixed_blending->isChecked();
    out << ui->actionReal_time_blending->isChecked();
}

/**
 * @brief MainWindow::exportResultDirect
 *
 * This slot exports the result image in the same file path as
 * the last exported result, without asking or showing a dialog.
 * If there was no previously exported file, this slot has the
 * same effect as exportResultAs().
 */
void MainWindow::exportResultDirect() {
    // Check if there is a previous export location
    if (m_last_export_filename.isEmpty()) {
        // If there is no previous location -> open file dialog
        exportResultAs();
        return;
    }

    // Export the blending
    exportBlendingResult(m_last_export_filename);
}

/**
 * @brief MainWindow::exportResultAs
 *
 * This slot asks for a export file location then exports
 * the blending result into it.
 */
void MainWindow::exportResultAs() {
    // Ask for export location
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Export the blending result as...",
                QDir::homePath(),
                IMAGE_WRITE_EXT);

    // If the save dialog was canceled
    if (filename.isEmpty())
        return;

    // Store this file location for future exportations
    m_last_export_filename = filename;

    // Export the blending result to this location
    exportBlendingResult(filename);
}

/**
 * @brief MainWindow::exportBlendingResult
 * @param filename
 *
 * This function exports the current blending result into filename.
 * Any invalid (not yet computed) pasted layer will be ignored
 */
void MainWindow::exportBlendingResult(QString filename) {
    // Create a copy of the target image
    QImage blended_image = m_target_image;

    // Create a painter device to draw the pasted layers over the background
    QPainter painter(&blended_image);

    // Loop over each pasted layer
    foreach (PastedSourceItem *item, m_scene_target->getSourceItemList()) {
        // Ignore invalid layers
        if (item->isInvalid() || item->isComputing())
            continue;

        // Draw the pasted layer's blended image to its position
        painter.drawImage(item->pos().toPoint(), item->blendedImage());
    }

    // Export the blended image into a file
    if (!blended_image.save(filename)) {
        // An error occurred
        QMessageBox::critical(
                    this,
                    "Blending exportation error",
                    "An error occurred while writing the blended image to the selected file.");
        return;
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

    // Enable lasso only if the source image is present
    m_scene_source->enableLasso(!m_source_image.isNull());
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

    updateUiComponents();

    QRect select_rect = m_scene_source->getSelectionPath().boundingRect().toAlignedRect();
    QString string_size = QString("Selection: %1x%2 px").arg(select_rect.width()).arg(select_rect.height());
    m_label_size->setText(string_size);
}

/**
 * @brief MainWindow::sourceLassoRemoved
 *
 * This slot is called when a lasso has been removed from the source scene
 */
void MainWindow::sourceLassoRemoved() {
    updateUiComponents();
    m_label_size->clear();
}

/**
 * @brief MainWindow::clearLassoSelection
 *
 * This slot clears the lasso selection
 */
void MainWindow::clearLassoSelection() {
    m_scene_source->removeLasso();
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
        if (event->modifiers() & Qt::ControlModifier) {
            // F5 with control key
            // -> recompute all the pasted items
            m_scene_target->recomputeBlendingAll();
        }
        else {
            // F5 without control key
            // -> recompute the selected item (if one)
            m_scene_target->recomputeBlendingSelected();
        }
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
 * @brief MainWindow::pastedItemListChanged
 *
 * This slot is called by the scene when the number of pasted items changes.
 */
void MainWindow::pastedItemListChanged() {
    // Get the number of pasted items
    int items_count = m_scene_target->getSourceItemList().size();

    // These actions are enabled only if at least an item has been pasted
    ui->actionDelete_all_layers->setEnabled(items_count > 0);
    ui->actionRecompute_all_layers->setEnabled(items_count > 0);
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
 * @brief MainWindow::updateUiComponents
 *
 * This slot updates the UI components state (enabled/disabled,...)
 */
void MainWindow::updateUiComponents() {
    ui->actionSave_project->setEnabled(!m_source_image.isNull() || !m_target_image.isNull());
    ui->actionExport->setEnabled(!m_target_image.isNull());
    ui->actionExport_as->setEnabled(!m_target_image.isNull());

    bool lasso_valid = m_scene_source->isSelectionValid();
    ui->actionClear_selection->setEnabled(lasso_valid);
    ui->transferButton->setEnabled(!m_target_image.isNull() && lasso_valid);
    ui->actionTransfer_selection->setEnabled(!m_target_image.isNull() && lasso_valid);
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

    // Get the selection lasso bounding rect aligned on the pixels
    QRect select_rect = m_scene_source->getSelectionPath().boundingRect().toAlignedRect();

    // Add 1px margin to the lasso bounding rect
    select_rect.adjust(-1, -1, 1, 1);

    // Check if the selection can enter in the target image
    if (!m_scene_target->isRectangleInsertable(select_rect)) {
        QMessageBox::critical(
                    this,
                    "Oversized source",
                    "The source item you are trying to paste is larger than the target.\n"
                    "Try again with a smaller source or a larger target.");

        return;
    }

    // Get a copy of the source image from inside the bounding rect
    QImage src_img_part = m_source_image.copy(select_rect);

    // Normalize the selection path to its bounding rect (with 1px margin)
    QPainterPath path = m_scene_source->getSelectionPath();

    // Create the Pasted Source Item
    PastedSourceItem *src_item = new PastedSourceItem(src_img_part, path, m_target_image);
    src_item->setRealTime(ui->actionReal_time_blending->isChecked());
    src_item->setMixedBlending(ui->actionMixed_blending->isChecked());

    // Add the source item to the target scene
    m_scene_target->addSourceItem(src_item);
}

