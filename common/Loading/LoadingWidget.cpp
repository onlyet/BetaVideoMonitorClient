#include <ui_LoadingWidget.h>
#include "LoadingWidget.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QMovie>
#include <QTimer>

LoadingWidget::LoadingWidget(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

    ui = new Ui::LoadingWidget();
    ui->setupUi(this);

    int w = QGuiApplication::primaryScreen()->availableGeometry().width();
    int h = QGuiApplication::primaryScreen()->availableGeometry().height();
    setFixedSize(w, h);
    ui->loadingLabel->setFixedSize(300, 300);

#ifdef SHOWBY
    m_movie = new QMovie(":/Image/loading2.gif");
#else
    m_movie = new QMovie(":/loading.gif");
#endif
    ui->loadingLabel->setMovie(m_movie);
}

LoadingWidget::~LoadingWidget()
{
}

void LoadingWidget::showLoadingShade()
{
    show();
    m_movie->start();
    raise();
}

void LoadingWidget::hideLoadingShade()
{
    hide();
    // �ӳ�stop���������ص�ʱ���о��α���
    //QTimer::singleShot(500, [this]() {
    //    m_movie->stop();
    //}); 
}

void LoadingWidget::paintEvent(QPaintEvent *ev)
{
    //Ϊ�������һ����͸���ľ�������
    //QPainter painter(this);
    //painter.fillRect(this->rect(), QColor(0, 0, 0, 100));
    QWidget::paintEvent(ev);
}
