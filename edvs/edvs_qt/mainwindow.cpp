#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boost/bind.hpp"
#include "stdio.h"
#include "stdint.h"
#include "qtimer.h"
#include "qimage.h"





const int cDecay = (2 * 256) / 60;
const int cDisplaySize = 512;
const int cUpdateInterval = FILTER_DIFF*2;
const Edvs::Baudrate cBaudrate = Edvs::B4000k;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->recCtrl->setText("Start");
    ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->label->setMinimumSize(400, 400);
    connect(ui->recCtrl, SIGNAL(clicked()), this, SLOT(recButtonClicked()));
    img = QImage(ui->label->width(), ui->label->height(), QImage::Format_Grayscale8);
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
    uint8_t pixel_index;

    int i;

    dat = active;
    active = active == DATA1 ? DATA2 : DATA1;
    if(__capture)
    {
        file = fopen("edvs_frames.dat", "ab");
        fwrite(dat, 1, DATA_LEN, file);
        fclose(file);
    }
    for(i = 0; i < DATA_LEN; i++)
    {
        pixel_index = dat[i]+33;
        img.setPixel(i % 128, i / 128, pixel_index);
    }
    img.scaled(ui->label->width(), ui->label->height());
    ui->label->setPixmap(QPixmap::fromImage(img));
}


void MainWindow::recButtonClicked()
{
    ui->recCtrl->setText(__capture ? "Start" : "Stop");
    __capture = !__capture;
}
