#include "imagegraphicsview.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QResizeEvent>
#include <QTransform>
#include <QImageReader>

#include <QDebug>

#define CHECKERBOARD_SIZE 10

ImageGraphicsView::ImageGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    // Enable mouse tracking
    setMouseTracking(true);

    // Enable antialiasing for the graphics view
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Generate the checkerboard background
    updateViewCheckerboard();

    // This widget accepts dropped files
    setAcceptDrops(true);
}

ImageGraphicsView::~ImageGraphicsView() {

}


void ImageGraphicsView::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);

    updateViewAdjustment();
}

/**
 * @brief ImageGraphicsView::updateViewCheckerboard
 *
 * This slot updates the checkerboard background of the view
 */
void ImageGraphicsView::updateViewCheckerboard() {
    // Define a checkerboard background for the graphics views
    QBrush checkerboard_brush(Qt::lightGray, Qt::Dense4Pattern);

    // We want a checkerboard that is invariant to the graphics view scale.
    // Create a tranformation matrix which is the inverse of the graphics view matrix.
    QTransform t = transform().inverted();

    // Zoom on the checkerboard to get large squares
    t.scale(CHECKERBOARD_SIZE, CHECKERBOARD_SIZE);

    // Apply the resulting transformation matrix to the brush
    checkerboard_brush.setTransform(t);

    // Apply the checkerboard brush
    setBackgroundBrush(checkerboard_brush);
}

/**
 * @brief ImageGraphicsView::updateViewAdjustment
 *
 * This slot fits the view on the scene rect (if a scene is set)
 */
void ImageGraphicsView::updateViewAdjustment() {
    if (!scene())
        return;

    // If a scene is set -> fit view on the scene rect
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);

    // The view has been resized -> update checkerboard
    updateViewCheckerboard();
}

/**
 * @brief ImageGraphicsView::setScene
 * @param scene
 *
 * This is an overloaded function.
 * It do the same as QGraphicsView::setScene(), but also connects the sceneRectChanged() signal
 * to fit the view and update the checkerboard.
 */
void ImageGraphicsView::setScene(QGraphicsScene *scene) {
    QGraphicsView::setScene(scene);

    connect(scene, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(updateViewAdjustment()));
}

/**
 * @brief ImageGraphicsView::dragEnterEvent
 * @param event
 *
 * This function is called when a dragged file is presented over the
 * widget's area. It must answer if the drop can be accepted or not.
 * In this case, we want to accept only files with an image extension.
 */
void ImageGraphicsView::dragEnterEvent(QDragEnterEvent *event) {
    // Accept the drag event only if the dragged file is an URL
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    // Get informations about the presented file
    QFileInfo f_info(event->mimeData()->urls().first().toLocalFile());

    // If this is not a file or it is not readable -> refuse
    if (!f_info.isFile() || !f_info.isReadable()) {
        return;
    }

    // Retrive supported image extensions list
    QList<QByteArray> supported_fmts = QImageReader::supportedImageFormats();

    // Check if the presented file extension is supported
    if (!supported_fmts.contains(f_info.suffix().toLower().toUtf8())) {
        return;
    }

    event->acceptProposedAction();
}

/**
 * @brief ImageGraphicsView::dragMoveEvent
 * @param event
 *
 * This function is called when a dragged file is dropped over the widget's
 * area. It is needed to accept this event in order to receive the dropEvent.
 */
void ImageGraphicsView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

/**
 * @brief ImageGraphicsView::dragMoveEvent
 * @param event
 *
 * This function is called when a dragged file is dropped over the widget's
 * area. It is needed to accept this event in order to receive the dropEvent.
 */
void ImageGraphicsView::dragLeaveEvent(QDragLeaveEvent *event) {
    event->accept();
}

/**
 * @brief ImageGraphicsView::dropEvent
 * @param event
 *
 * This function is called when a file has been dropped in the widget's area.
 * If the dropped file is a valid image path, a signal is emitted.
 * In this case, the dragEnterEvent has been accepted for image files only.
 * Therefore, the dropped file can only be a supported image file.
 */
void ImageGraphicsView::dropEvent(QDropEvent *event) {
    event->acceptProposedAction();

    // Get the presented filename
    QString filename = event->mimeData()->urls().first().toLocalFile();

    // Emit a signal
    emit imageFileDropped(filename);
}

