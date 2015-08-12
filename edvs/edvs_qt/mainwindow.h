#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Edvs.h"
#include <vector>
#include "opencv2/features2d.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "tracker.h"



#define DATA1       	data_buffer[0]
#define DATA2       	data_buffer[1]
#define DATA_LEN    	128*128
#define LED_FREQUENCY   1000    /*Hz*/
#define FILTER_DIFF 	10
#define TOLERANCE       20
#define UPDATE_INTERVAL 20


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void OnEvent(const std::vector<Edvs::Event>& events);

public slots:
    void update_timer();
    void recButtonClicked();

private:
    Ui::MainWindow *ui;

    uint8_t data_buffer[2][DATA_LEN];
    uint8_t *active;

    Edvs::Device device;
    Edvs::EventCapture capture;
    QTimer *timer;
    QImage img;
    cv::Ptr<cv::ORB> _orb;
    neurocatch::Tracker tracker;

    bool __capture;
};

#endif // MAINWINDOW_H
