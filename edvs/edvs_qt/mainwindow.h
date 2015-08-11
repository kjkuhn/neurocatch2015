#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Edvs.h"
#include <vector>


#define DATA1       	data_buffer[0]
#define DATA2       	data_buffer[1]
#define DATA_LEN    	128*128
#define LED_FREQUENCY   1000    /*Hz*/
#define FILTER_DIFF 	10
#define TOLERANCE       20
#define UPDATE_INTERVAL 10


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

    bool __capture;
};

#endif // MAINWINDOW_H
