#include "imagegraphicsview.h"

#include <QResizeEvent>
#include <QTransform>

#define CHECKERBOARD_SIZE 10

ImageGraphicsView::ImageGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    // Enable mouse tracking
    setMouseTracking(true);

    // Enable antialiasing for the graphics view
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Generate the checkerboard background
    updateViewCheckerboard();
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
