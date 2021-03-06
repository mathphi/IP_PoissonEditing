#ifndef SOURCEGRAPHICSSCENE_H
#define SOURCEGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QPainterPath>


namespace SourceLassoState {
enum SourceLassoState {
    Disabled,
    NoLasso,
    Drawing,
    Drawn
};
}


class GraphicsLassoItem;

class SourceGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    SourceGraphicsScene(QObject *parent = nullptr);
    ~SourceGraphicsScene();

    QPainterPath getSelectionPath() const;
    bool isSelectionValid() const;

public slots:
    void enableLasso(bool en);
    void removeLasso();

signals:
    void lassoDrawn(const QPainterPath);
    void lassoRemoved();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    GraphicsLassoItem *m_current_lasso;
    SourceLassoState::SourceLassoState m_lasso_state;
};

#endif // SOURCEGRAPHICSSCENE_H
