#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTimer"
#include "atomic"


#define MY_SIZE             75
#define UPDATE_INTERVAL     1
#define WINDOW_SIZE         750

#define MY_RECT(poly, x, y)  \
    poly << QPoint(x,y) << QPoint(x+MY_SIZE,y) << QPoint(x+MY_SIZE,y+MY_SIZE) << QPoint(x,y+MY_SIZE) << QPoint(x,y)

#define MY_TRIANGLE(poly, x, y) \
    poly << QPoint(x,y+MY_SIZE) << QPoint(x+MY_SIZE,y+MY_SIZE) << QPoint(x+(MY_SIZE/2),y) << QPoint(x,y+MY_SIZE)


std::atomic_int trix,triy, trixtarget, triytarget;
std::atomic_int rectx,recty,rectxtarget,rectytarget;
std::atomic_int state;
QTimer *tim;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    trix = 0;
    triy = 0;
    rectx = 500;
    recty = 500;
    trixtarget = random()%WINDOW_SIZE;
    triytarget = random()%WINDOW_SIZE;
    rectxtarget = random()%WINDOW_SIZE;
    rectytarget = random()%WINDOW_SIZE;
    state = 0;
    tim = new QTimer(this);
    connect(tim, SIGNAL(timeout()), this, SLOT(timedOut()));
    tim->start(UPDATE_INTERVAL);
    ui->centralWidget->setMinimumSize(WINDOW_SIZE,WINDOW_SIZE);
    ui->centralWidget->setMinimumSize(WINDOW_SIZE,WINDOW_SIZE);

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::mousePressEvent(QMouseEvent *e)
{
    //add object
    if(e->button() == Qt::LeftButton)
    {
        state = (state+1)%4;
    }
    //remove object
    else if(e->button() == Qt::RightButton)
    {
        state = state > 0?state-1:3;
    }
}




void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black);
    QBrush brush;
    QPainterPath path;
    QPolygon rect, tri;

    MY_TRIANGLE(tri, trix,triy);
    MY_RECT(rect, rectx, recty);

    brush.setColor(Qt::green);
    brush.setStyle(Qt::SolidPattern);
    pen.setWidth(23);
    painter.setPen(pen);
    //painter.drawEllipse(QPoint(100,100),50,50);

    switch(state)
    {
    case 0:break;
    case 1:
        path.addPolygon(rect);
        painter.drawPolygon(rect);
        break;
    case 2:
        path.addPolygon(tri);
        painter.drawPolygon(tri);
        break;
    case 3:
        path.addPolygon(rect);
        painter.drawPolygon(rect);
        path.addPolygon(tri);
        painter.drawPolygon(tri);
        break;
    }
    painter.fillPath(path, brush);
}


void MainWindow::timedOut()
{
    int rx,ry,tx,ty;
    rx = rectxtarget-rectx;
    ry = rectytarget-recty;
    tx = trixtarget-trix;
    ty = triytarget-triy;
    if(rx < 0)
        rectx--;
    else if(rx > 0)
        rectx++;
    if(ry < 0)
        recty--;
    else if(ry > 0)
        recty++;
    if(rx == 0 && ry == 0)
    {
        rectxtarget = random()%WINDOW_SIZE;
        rectytarget = random()%WINDOW_SIZE;
    }

    if(tx < 0)
        trix--;
    else if(tx > 0)
        trix++;
    if(ty < 0)
        triy--;
    else if(ty > 0)
        triy++;
    if(ty == 0 && tx == 0)
    {
        trixtarget = random()%WINDOW_SIZE;
        triytarget = random()%WINDOW_SIZE;
    }
    update();
}
