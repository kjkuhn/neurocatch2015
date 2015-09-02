#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boost/bind.hpp"
#include "stdio.h"
#include "stdint.h"
#include "qtimer.h"
#include "qimage.h"





#define DATA1       	data_buffer[0]
#define DATA2       	data_buffer[1]





const Edvs::Baudrate cBaudrate = Edvs::B4000k;
FILE *_file;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->recCtrl->setText("Start");
    img = QImage(Edvs::cDeviceDisplaySize/*2448*/, /*3264*/Edvs::cDeviceDisplaySize, QImage::Format_RGB32);
#if !DISPLAY_ONLY
    tracker = new neurocatch::Tracker();
    connect(tracker, &neurocatch::Tracker::sendFrame, this, &MainWindow::update_key_label);
    connect(tracker, &neurocatch::Tracker::send_info, this, &MainWindow::update_info);
#if SLIDING_FRAMES
    frame_mgr = new neurocatch::FrameManager();
#elif USE_DYNAMIC_BB
    frame_mgr = new neurocatch::FrameManager(tracker);
    connect(frame_mgr, &neurocatch::FrameManager::update_img, this, &MainWindow::update_img);
#endif /*SLIDING_FRAMES/USE_DYNAMIC_ORB*/
#endif /*!DISPLAY_ONLY*/
    __capture = false;
#if !DEBUG  ||  DISPLAY_ONLY
    ui->keys->hide();
#else
    ui->keys->setFixedSize(DISPLAY_FRAME_WIDTH, DISPLAY_FRAME_HEIGHT);
#endif /*!DEBUG  ||  DISPLAY_ONLY*/
    ui->label->setFixedSize(DISPLAY_FRAME_WIDTH, DISPLAY_FRAME_HEIGHT);
#if ONLINE
    connect(ui->recCtrl, SIGNAL(clicked()), this, SLOT(recButtonClicked()));
    memset(DATA1, 0, DATA_LEN);
    memset(DATA2, 0, DATA_LEN);
    active = DATA1;
    device = Edvs::Device(cBaudrate);
    capture = Edvs::EventCapture(device, boost::bind(&MainWindow::OnEvent, this, _1));
    _file = fopen(EDVS_OUT_FILE, "wb");
    fclose(_file);
#else
    _file = fopen(OFFLINE_FILE,"r");
    ui->recCtrl->hide();
#endif /*ONLINE*/
#if USE_SPHERO
    connect(ui->sCtrl, SIGNAL(clicked()), this, SLOT(spheroButtonClicked()));
    connect(tracker->sphero, &neurocatch::SpheroController::position_reached, this, &MainWindow::spheroPositionReached);
#else
    ui->sCtrl->hide();
#endif /*USE_SPHERO*/
#if !USE_DYNAMIC_BB
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_timer()));
    timer->start(UPDATE_INTERVAL);
#endif /*!USE_DYNAMIC_ORB*/
    ui->time->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

static FILE *raw_file;

void MainWindow::OnEvent(const std::vector<Edvs::Event>& events)
{
    raw_file = fopen("raw_edvs.dat", "ab");
    for(std::vector<Edvs::Event>::const_iterator it=events.begin(); it!=events.end(); it++) {
#if SLIDING_FRAMES
       frame_mgr->push_evt(it->y*128+it->x);
#elif USE_DYNAMIC_BB
        frame_mgr->push_evt(it->y*128+it->x, it->parity);
#else
        active[it->y*128+it->x] += (*it).parity ? EDVS_ON_EVT : EDVS_OFF_EVT;
#endif /*!SLIDING_FRAMES*/
        fwrite(&(*it), sizeof(Edvs::Event), 1, raw_file);
    }
    fclose(raw_file);
}


void MainWindow::update_key_label(QImage *img)
{
    ui->keys->setPixmap(QPixmap::fromImage(img->scaled(ui->keys->width(), ui->keys->height())));
}


void MainWindow::update_info(const char *str)
{
    ui->info->setText(str);
}


void MainWindow::update_timer()
{


    int i;
#if ONLINE
    uint8_t *dat;
    FILE *file;

#if !SLIDING_FRAMES
    dat = active;
    active = active == DATA1 ? DATA2 : DATA1;
#else
    dat = frame_mgr->next();
#endif /*!SLIDING_FRAMES*/
    for(i = 0; i < DATA_LEN; i++)
    {
        if((char)dat[i] < 0)
            dat[i] = 100;
        else if((char)dat[i] > 0)
            dat[i] = 250;
        img.setPixel(i % 128, i / 128, qRgb(dat[i]==100?255:0, dat[i]==250?255:0, 0));
        //dat[i] = dat[i] == 0 ? 0 : dat[i] + 200;
        //img.setPixel(i % 128, i / 128, qRgb(dat[i], dat[i], dat[i]));
    }
    if(__capture)
    {
        file = fopen(EDVS_OUT_FILE, "ab");
        fwrite(dat, 1, DATA_LEN, file);
        fclose(file);
    }
#else
    uint8_t dat[DATA_LEN];
    if(feof(_file))
        freopen(OFFLINE_FILE, "r", _file);
    fread(dat, 1, DATA_LEN, _file);
    for(i = 0; i < DATA_LEN; i++)
    {
        if((char)dat[i] < 0)
            dat[i] = 100;
        else if((char)dat[i] > 0)
            dat[i] = 250;
        img.setPixel(i % 128, i / 128, qRgb(dat[i]==100?255:0, dat[i]==250?255:0, 0));

        //dat[i] = dat[i] == 0 ? 0 : dat[i] + 200;
        //img.setPixel(i % 128, i / 128, q20Rgb(dat[i], dat[i], dat[i]));
    }
#endif /*ONLINE*/

#if !DISPLAY_ONLY
    tracker->add_to_wl(dat);
#endif /*!DISPLAY_ONLY*/

    memset(dat, 0, DATA_LEN);
    ui->label->setPixmap(QPixmap::fromImage(img.scaled(ui->label->width(), ui->label->height())));
}


void MainWindow::recButtonClicked()
{
    ui->recCtrl->setText(__capture ? "Start" : "Stop");
    __capture = !__capture;
}


void MainWindow::spheroButtonClicked()
{
    tracker->sphero->signal_next_pos();
    ui->sCtrl->setDisabled(true);
}


void MainWindow::spheroPositionReached()
{
    ui->sCtrl->setDisabled(false);
}


void MainWindow::update_img(QImage *img)
{
    ui->label->setPixmap(QPixmap::fromImage(img->scaled(ui->label->width(), ui->label->height())));
}
