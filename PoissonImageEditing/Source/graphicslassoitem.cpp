#include "graphicslassoitem.h"

#include <QTimer>
#include <QObject>
#include <QPainter>

#include <QDebug>

#define LASSO_WIDTH     1.5
#define DASH_SIZE       6.0
#define ANIM_INTERVAL   250   // ms

GraphicsLassoItem::GraphicsLassoItem(const QPointF start_point) : QGraphicsObject()
{
    // Lasso is initially not terminated
    m_lasso_terminated = false;

    // Initialize painter path
    m_path = QPainterPath(start_point);
    m_path.setFillRule(Qt::WindingFill);    // To keep only path outline when simplifying

    // Initialize animation timer
    m_anim_timer = new QTimer;
    m_anim_timer->setInterval(ANIM_INTERVAL);
    m_anim_timer->start();

    // Dash offset (will be dynamically updated by timer)
    m_anim_dash_offset = 0.0;

    connect(m_anim_timer, SIGNAL(timeout()), this, SLOT(animateLasso()));
}

GraphicsLassoItem::~GraphicsLassoItem() {
    m_anim_timer->stop();
    delete m_anim_timer;
}

/**
 * @brief GraphicsLassoItem::animateLasso
 *
 * This slot is called by the timer to animate the dashes on the lasso
 */
void GraphicsLassoItem::animateLasso() {
    // Increase by an eighth of a period
    m_anim_dash_offset += DASH_SIZE / 4.0;

    // Increase dash offset with a period of 2*DASH_SIZE
    if (m_anim_dash_offset >= 2.0*DASH_SIZE) {
        m_anim_dash_offset = 0.0;
    }

    update();
}

/**
 * @brief GraphicsLassoItem::path
 * @return
 *
 * This function returns a copy of the current path of the lasso
 */
QPainterPath GraphicsLassoItem::path() const {
    return m_path;
}

/**
 * @brief GraphicsLassoItem::addPathPoint
 * @param point
 *
 * This function appends a point to the lasso's path
 */
void GraphicsLassoItem::addPathPoint(const QPointF point) {
    prepareGeometryChange();
    m_path.lineTo(point);
    update();
}

/**
 * @brief GraphicsLassoItem::terminateLasso
 *
 * This function terminates the lasso (closes the lasso and simplifies its geometry)
 */
void GraphicsLassoItem::terminateLasso() {
    prepareGeometryChange();
    m_path = m_path.simplified();
    update();

    m_lasso_terminated = true;
}

/**
 * @brief GraphicsLassoItem::isLassoValid
 * @return
 *
 * This function returns true if the lasso is a valid closed path
 */
bool GraphicsLassoItem::isLassoValid() const {
    // Check if first point == last point
    return m_path.pointAtPercent(0) == m_path.pointAtPercent(1);
}


QRectF GraphicsLassoItem::boundingRect() const {
    return m_path.boundingRect();
}

QPainterPath GraphicsLassoItem::shape() const {
    // Take the width of the pen into account
    QPainterPathStroker ps;
    ps.setWidth(LASSO_WIDTH);

    QPainterPath p = ps.createStroke(m_path);
    p.addPath(m_path);

    return p;
}

void GraphicsLassoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Get the scene scale
    qreal scene_scale = painter->deviceTransform().m11();

    // Draw the lasso with a multicolor dashed line (black and white)
    QPen pen;
    pen.setWidthF(LASSO_WIDTH/scene_scale);

    // Black dashed line (can be red if lasso invalid)
    pen.setColor(m_lasso_terminated && !isLassoValid() ? Qt::red : Qt::black);
    pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
    pen.setDashOffset(-m_anim_dash_offset);
    painter->setPen(pen);
    painter->drawPath(m_path);

    // White dashed line
    pen.setColor(Qt::white);
    pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
    pen.setDashOffset(-m_anim_dash_offset + 1.0 * DASH_SIZE);   // + Offset w.r.t. Black dashes
    painter->setPen(pen);
    painter->drawPath(m_path);

    // Fill the polygon if closed
    if (isLassoValid()) {
        pen.setColor(Qt::transparent);
        painter->setPen(pen);
        painter->setBrush(QBrush(QColor(0,0,0,50)));
        painter->drawPolygon(m_path.toFillPolygon());
    }
}
