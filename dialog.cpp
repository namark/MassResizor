#include "dialog.h"
#include "ui_dialog.h"
#include <QtConcurrent/QtConcurrent>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowSystemMenuHint |
                   Qt::WindowMinMaxButtonsHint |
                   Qt::WindowCloseButtonHint);

    file_selector = new QtUIUtils::FileSelector(this,
                                                ui->list_input, ui->btn_add, ui->btn_remove,
                                                "Select Images",
                                                "Image Files (*.png *.jpg *.jpeg *.bmp *.xpm *.xbm *.ppm)");

    connect(ui->btn_go, SIGNAL(clicked()), this, SLOT(go()));
    connect(ui->btn_output, SIGNAL(clicked()), this, SLOT(selectDir()));

    ui->slider_quality->setRange(MIN_QUALITY, MAX_QUALITY);
    ui->spin_quality->setRange(MIN_QUALITY, MAX_QUALITY);
    connect(ui->spin_quality, SIGNAL(valueChanged(int)), ui->slider_quality, SLOT(setValue(int)));
    connect(ui->slider_quality, SIGNAL(valueChanged(int)), ui->spin_quality, SLOT(setValue(int)));
    ui->spin_quality->setValue(100);

    ui->radio_width_height->setId(ui->radio_width,SCALE_TO_WIDTH);
    ui->radio_width_height->setId(ui->radio_height,SCALE_TO_HEIGHT);

    ui->progress->setValue(0);
    progressTimer = new QTimer();
    progress = 0;
    connect(progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()));

    ui->spin_width->setMaximum(0x7FFFFFFF);
    ui->spin_height->setMaximum(0x7FFFFFFF);

    ui->cambo_width->insertItem(PIXEL_UNIT,"pixels");
    ui->cambo_width->insertItem(PERSENTAGE_UNIT,"persentage");
    // should be a better way
    ui->cambo_height->insertItem(PIXEL_UNIT,"pixels");
    ui->cambo_height->insertItem(PERSENTAGE_UNIT,"persentage");

    check_groups << new QtUIUtils::CheckEnableGroup(ui->check_same_name, true);
    check_groups.last()->add(ui->ledit_prefix);
    check_groups.last()->add(ui->ledit_postfix);
    check_groups << new QtUIUtils::CheckEnableGroup(ui->check_next_to_input, true);
    check_groups.last()->add(ui->ledit_output);
    check_groups.last()->add(ui->btn_output);
    check_groups << new QtUIUtils::CheckEnableGroup(ui->radio_width);
    check_groups.last()->add(ui->cambo_width);
    check_groups.last()->add(ui->spin_width);
    check_groups << new QtUIUtils::CheckEnableGroup(ui->radio_height);
    check_groups.last()->add(ui->cambo_height);
    check_groups.last()->add(ui->spin_height);

    logger.set(ui->log_output, ui->toggle_log);
    logger << "hi!" << "i'll tell you stuff...";

    ui->toggle_log->setChecked(true);// force toggle signal
    ui->toggle_log->setChecked(false);
    ui->radio_width->setChecked(true);


    enableUI();
}

Dialog::~Dialog()
{
    while(!check_groups.empty())
        delete check_groups.takeLast();
    //delete file_selector;
    delete progressTimer;
    delete ui;
}

void Dialog::resizeImages()
{

    QString prefix,
            postfix;
    if(!ui->check_same_name->isChecked())
    {
        prefix = ui->ledit_prefix->text();
        postfix =ui->ledit_postfix->text();
    }
    else
    {
        prefix = "";
        postfix = "";
    }

    QList<QtUIUtils::FileName> filenames = file_selector->getFilenames();
    for(int i = 0, l = filenames.length(); i<l; ++i, progress = i*100/l)
    {
        const QtUIUtils::FileName& input_fn = filenames.at(i);
        QtUIUtils::FileName output_fn = input_fn;
        // NOTE: add extensions to filename
        output_fn.setName(prefix+output_fn.name()+postfix);
        if(!ui->check_next_to_input->isChecked())
            output_fn.setPath(ui->ledit_output->text() + output_fn.detected_separator());
        if(!_im.load(input_fn))
        {
            logger != "Failed to read " + input_fn.name();
            continue;
        }
        Qt::TransformationMode tmode = ui->check_bilinear->isChecked()
                ? Qt::FastTransformation
                : Qt::SmoothTransformation;
        switch(ui->radio_width_height->checkedId())
        {
        case SCALE_TO_WIDTH:
            _im = _im.scaledToWidth(targetWidth(), tmode);
        break;
        case SCALE_TO_HEIGHT:
            _im = _im.scaledToHeight(targetHeight(), tmode);
        break;
        }
        if(!_im.save(output_fn, 0, ui->spin_quality->value()))
        {
            logger != "Failed to write  " + output_fn.name();
            continue;
        }
        logger << input_fn.fullName() + " -> " + output_fn.fullName();
    }

    progress = 113; // aka let it know you finnished
}

void Dialog::updateProgress()
{
    ui->progress->setValue(progress);
    logger.update();
    if(progress == 113) // aka thread finished
    {
        logger.autoUpdate(true);
        ui->progress->setValue(100);
        progressTimer->stop();
        enableUI();
    }
}

void Dialog::go()
{
    progress = 0;
    updateProgress();
    enableUI(false);
    progressTimer->start(100);
    logger.autoUpdate(false);
    QtConcurrent::run(this, &Dialog::resizeImages);
}

void Dialog::selectDir()
{
    ui->ledit_output->setText(QFileDialog::getExistingDirectory(this, "Select output directory"));
}

void Dialog::enableUI(bool enable)
{
    UIEnabled = enable;
    // enable/disable UI
    file_selector->enable(enable);
    ui->btn_go->setEnabled(enable);
    ui->check_bilinear->setEnabled(enable);
    ui->slider_quality->setEnabled(enable);
    ui->spin_quality->setEnabled(enable);

    for(int i=0, l=check_groups.length(); i<l; ++i)
        check_groups[i]->enable(enable);
}

int Dialog::targetWidth()
{
    int value = ui->spin_width->value();
    switch(ui->cambo_width->currentIndex())
    {
    case PIXEL_UNIT:
        return value;
    break;
    case PERSENTAGE_UNIT:
        return round(_im.width() * value * 0.01); // alright?
    break;
    default: return -1;
    }
}

int Dialog::targetHeight()
{
    int value = ui->spin_height->value();
    switch(ui->cambo_height->currentIndex())
    {
    case PIXEL_UNIT:
        return value;
    break;
    case PERSENTAGE_UNIT:
        return round(_im.height() * value * 0.01); // alright?
    break;
    default: return -1;
    }
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    if(UIEnabled && e->key() == Qt::Key_Escape) close();
}
