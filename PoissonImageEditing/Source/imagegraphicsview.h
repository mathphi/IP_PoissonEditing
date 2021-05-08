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

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void imageFileDropped(QString);

private slots:
    void updateViewCheckerboard();
    void updateViewAdjustment();
};

#endif // IMAGEGRAPHICSVIEW_H
