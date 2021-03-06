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
#include "framemanager.h"
#include "settings.h"


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
    void update_key_label(QImage *img);
    void update_info(const char *str);
    void spheroButtonClicked();
    void spheroPositionReached();
    void update_img(QImage *img);

private:
    Ui::MainWindow *ui;

    uint8_t data_buffer[2][DATA_LEN];
    uint8_t *active;

    Edvs::Device device;
    Edvs::EventCapture capture;
    QTimer *timer;
    QImage img;
    neurocatch::Tracker *tracker;
    neurocatch::FrameManager *frame_mgr;

    bool __capture;
};

#endif // MAINWINDOW_H
