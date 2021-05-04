#ifndef SOURCEGRAPHICSSCENE_H
#define SOURCEGRAPHICSSCENE_H

#include <QGraphicsScene>


namespace SourceLassoState {
enum SourceLassoState {
    None,
    Drawing,
    Drawn
};
}


class GraphicsLassoItem;

class SourceGraphicsScene : public QGraphicsScene
{
public:
    SourceGraphicsScene(QObject *parent = nullptr);
    ~SourceGraphicsScene();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    void removeLasso();

    GraphicsLassoItem *m_current_lasso;
    SourceLassoState::SourceLassoState m_lasso_state;
};

#endif // SOURCEGRAPHICSSCENE_H
