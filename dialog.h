#ifndef DIALOG_H
#define DIALOG_H

#include <QtUIUtils/fileselector.h>
#include <QtUIUtils/checkenablegroup.h>
#include <QtUIUtils/checkvisibilitygroup.h>
#include <QtUIUtils/logger.h>
#include <QDialog>
#include <QKeyEvent>
#include <QFuture>
#include <QTimer>
#include <QImage>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:

    static const int MIN_QUALITY = 0;
    static const int MAX_QUALITY = 100;
    enum ScaleMethod {
        SCALE_TO_WIDTH,
        SCALE_TO_HEIGHT
    };

    enum ScaleUnitType {
        PIXEL_UNIT,
        PERSENTAGE_UNIT
    };

    Ui::Dialog *ui;
    int progress;
    QImage _im;

    QTimer* progressTimer;
    bool UIEnabled;

    QtUIUtils::FileSelector *file_selector;
    QList<QtUIUtils::CheckGroup*> check_groups;
    QtUIUtils::Logger logger;

    void resizeImages();
    void enableUI(bool = true);
    int targetWidth();
    int targetHeight();

    void keyPressEvent(QKeyEvent*);

private slots:
    void go();
    void selectDir();
    void updateProgress();
};

#endif // DIALOG_H
