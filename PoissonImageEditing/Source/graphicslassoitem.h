#ifndef GRAPHICSLASSOITEM_H
#define GRAPHICSLASSOITEM_H

#include <QGraphicsObject>

class GraphicsLassoItem : public QGraphicsObject
{
    Q_OBJECT

public:
    GraphicsLassoItem(const QPointF start_point);
    ~GraphicsLassoItem();

    QPainterPath path() const;
    void addPathPoint(const QPointF point);
    void terminateLasso(QRectF boundary_rect = QRectF());

    bool isLassoValid() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private slots:
    void animateLasso();

private:
    QPainterPath m_path;

    QTimer *m_anim_timer;
    qreal m_anim_dash_offset;

    bool m_lasso_terminated;
};

#endif // GRAPHICSLASSOITEM_H
