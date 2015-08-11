#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boost/bind.hpp"
#include "stdio.h"
#include "stdint.h"
#include "qtimer.h"
#include "qimage.h"




const Edvs::Baudrate cBaudrate = Edvs::B4000k;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->recCtrl->setText("Start");
    _orb = cv::ORB::create(500, 2, 8, 2, 0,4,cv::ORB::FAST_SCORE, 2, 20);
    connect(ui->recCtrl, SIGNAL(clicked()), this, SLOT(recButtonClicked()));
    img = QImage(Edvs::cDeviceDisplaySize/*2448*/, /*3264*/Edvs::cDeviceDisplaySize, QImage::Format_RGB32);
    __capture = false;
    memset(DATA1, 0, DATA_LEN);
    memset(DATA2, 0, DATA_LEN);
    active = DATA1;
    device = Edvs::Device(cBaudrate);
    capture = Edvs::EventCapture(device, boost::bind(&MainWindow::OnEvent, this, _1));
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


void MainWindow::update_timer()
{
    FILE *file;
    uint8_t *dat;
    cv::Mat mat, desc;
    std::vector<cv::KeyPoint> kp;
    cv::KeyPoint *k;

    int i,x,y;

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


void MainWindow::recButtonClicked()
{
    ui->recCtrl->setText(__capture ? "Start" : "Stop");
    __capture = !__capture;
}
