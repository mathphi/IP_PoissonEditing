#ifndef IMAGEGRAPHICSVIEW_H
#define IMAGEGRAPHICSVIEW_H

#include <QGraphicsView>

class ImageGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    ImageGraphicsView(QWidget *parent = nullptr);
    ~ImageGraphicsView();

    void setScene(QGraphicsScene *scene);

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateViewCheckerboard();
    void updateViewAdjustment();
};

#endif // IMAGEGRAPHICSVIEW_H
