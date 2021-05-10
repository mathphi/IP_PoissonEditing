#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainterPath>
#include <QLabel>

class QGraphicsScene;
class QGraphicsPixmapItem;

class SourceGraphicsScene;
class TargetGraphicsScene;

class ComputationHandler;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // I/O handling slots
    void openSourceImage(QString filename = QString());
    void openTargetImage(QString filename = QString());

    void exportResultDirect();
    void exportResultAs();

    // Help action slots
    void aboutQtDialog();
    void aboutProgramDialog();

    // Transfer from source to target
    void transferLassoSelection();

    // Source scene related slots
    void updateSourceScene();
    void sourceLassoDrawn(QPainterPath path);
    void sourceLassoRemoved();
    void clearLassoSelection();

    // Target scene related slots
    void updateTargetScene();
    void targetSceneKeyPressed(QKeyEvent *event);
    void targetSceneSelectionChanged();
    void pastedItemListChanged();
    void askRemoveAllLayers();

private:
    void exportBlendingResult(QString filename);

    Ui::MainWindow *ui;

    QStatusBar *m_status_bar;
    QLabel     *m_label_size;

    SourceGraphicsScene *m_scene_source;
    TargetGraphicsScene *m_scene_target;

    QGraphicsPixmapItem *m_pix_item_source;
    QGraphicsPixmapItem *m_pix_item_target;

    QImage m_source_image;
    QImage m_target_image;

    ComputationHandler *m_computation_handler;

    QString m_last_export_filename;
};
#endif // MAINWINDOW_H
