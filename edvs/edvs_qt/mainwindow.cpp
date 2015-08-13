#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boost/bind.hpp"
#include "stdio.h"
#include "stdint.h"
#include "qtimer.h"
#include "qimage.h"






const Edvs::Baudrate cBaudrate = Edvs::B4000k;
FILE *_file;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->recCtrl->setText("Start");
    _orb = cv::ORB::create(500, 2, 8, 2, 0,4,cv::ORB::FAST_SCORE, 2, 20);

    img = QImage(Edvs::cDeviceDisplaySize/*2448*/, /*3264*/Edvs::cDeviceDisplaySize, QImage::Format_RGB32);
    tracker = new neurocatch::Tracker();
    connect(tracker, &neurocatch::Tracker::sendFrame, this, &MainWindow::update_key_label);
    __capture = false;
#if !DEBUG
    ui->keys->hide();
#else
    ui->keys->setMinimumHeight(500);
    ui->keys->setMidLineWidth(500);
#endif
    ui->label->setMinimumHeight(500);
    ui->label->setMidLineWidth(500);
#if ONLINE
    connect(ui->recCtrl, SIGNAL(clicked()), this, SLOT(recButtonClicked()));
    memset(DATA1, 0, DATA_LEN);
    memset(DATA2, 0, DATA_LEN);
    active = DATA1;
    device = Edvs::Device(cBaudrate);
    capture = Edvs::EventCapture(device, boost::bind(&MainWindow::OnEvent, this, _1));
#else
    _file = fopen(FILE_NAME,"r");
    ui->recCtrl->hide();
#endif
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_timer()));
    timer->start(UPDATE_INTERVAL);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::OnEvent(const std::vector<Edvs::Event>& events)
{
    for(std::vector<Edvs::Event>::const_iterator it=events.begin(); it!=events.end(); it++) {
       active[it->y*128+it->x] += 1;
    }
}


void MainWindow::update_key_label(QImage *img)
{
    ui->keys->setPixmap(QPixmap::fromImage(img->scaled(ui->keys->width(), ui->keys->height())));
}


#ifndef __OLD
void MainWindow::update_timer()
{
    FILE *file;

    cv::Mat mat, desc;
    std::vector<cv::KeyPoint> kp;
    cv::KeyPoint *k;

    int i,x,y;
#if ONLINE
    uint8_t *dat;

    dat = active;
    active = active == DATA1 ? DATA2 : DATA1;
    for(i = 0; i < DATA_LEN; i++)
    {
        dat[i] = dat[i] == 0 ? 0 : dat[i] + 200;
        //img.setPixel(i % 128, i / 128, qRgb(dat[i], dat[i], dat[i]));
    }
    if(__capture)
    {
        file = fopen("edvs_frames.dat", "ab");
        fwrite(dat, 1, DATA_LEN, file);
        fclose(file);
    }
#else
    uint8_t dat[DATA_LEN];
    if(feof(_file))
        freopen(FILE_NAME, "r", _file);
    fread(dat, 1, DATA_LEN, _file);
#endif
    tracker->add_to_wl(dat);
    mat = cv::Mat(128,128,CV_8UC1, dat);

    _orb.get()->detect(mat, kp);
    //_orb.get()->compute(mat, kp, desc);

    //for(std::vector<cv::KeyPoint>::iterator it = kp.begin(); it != kp.end(); it++)
    //    img.setPixel(static_cast<int>(it->pt.x), static_cast<int>(it->pt.y), qRgb(217,255,0));
    cv::drawKeypoints(mat, kp, desc, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    for(y = 0; y < desc.rows; y++)
    {
        for(x = 0; x < desc.cols; x++){
            img.setPixel(x,y,qRgb(desc.at<uint8_t>(y,x),desc.at<uint8_t>(y,x),desc.at<uint8_t>(y,x)));
        }
    }
    //_orb.get()->compute(mat, kp, desc);
    /*if(!kp.empty())
    {
        k = (cv::KeyPoint*)&(*kp.begin());
        printf("x: %f\ty: %f", k->pt.x,k->pt.y);
    }
    */
    memset(dat, 0, DATA_LEN);
    ui->label->setPixmap(QPixmap::fromImage(img.scaled(ui->label->width(), ui->label->height())));
}

#else

void MainWindow::update_timer()
{

    uint8_t dat[128*128];
    cv::Mat mat, desc;
    std::vector<cv::KeyPoint> kp;
    cv::KeyPoint *k;

    if(feof(file))
        fseek(_file, 0, SEEK_SET);
    fread(dat, 1, DATA_LEN, _file);
    for(int i = 0; i < DATA_LEN; i++)
    {
        dat[i] = dat[i] == 0 ? 0 : dat[i] + 200;
        img.setPixel(i % 128, i / 128, qRgb(dat[i], dat[i], dat[i]));
    }
    ui->label->setPixmap(QPixmap::fromImage(img.scaled(ui->label->width(), ui->label->height())));
}

#endif


void MainWindow::recButtonClicked()
{
    ui->recCtrl->setText(__capture ? "Start" : "Stop");
    __capture = !__capture;
}
