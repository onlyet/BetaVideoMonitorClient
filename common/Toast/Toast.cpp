#include "ui_Toast.h"
#include "Toast.h"
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>

Toast::Toast(const QString &msg, QWidget *parent, int duration)
	: QWidget(parent)
    , ui(new Ui::Toast)
{
    ui->setupUi(this);
#ifdef SHOWBY
    ui->toastLabel->setStyleSheet("QLabel{color:rgb(28, 69, 152);}");
#endif // SHOWBY


    if (parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }
    else
    {

    }
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint/* | Qt::Tool*/);
    ui->toastLabel->setText(msg);

	m_animation = new QPropertyAnimation(this, "windowOpacity");
	connect(m_animation, SIGNAL(finished()), this, SLOT(close()));
	connect(m_animation, SIGNAL(finished()), this, SLOT(deleteLater()));
	m_animation->setDuration(duration);
	m_animation->setStartValue(1);
	m_animation->setEndValue(0.6);
	m_animation->setEasingCurve(QEasingCurve::OutInExpo);
    // FIXME: 有bug，暂时不播放动画
	m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Toast::showMessage(const QString &msg, QWidget *parent /* = nullptr */, int msecs /* = 2000 */)
{
	Toast *toast = new Toast(msg, parent, msecs);
	if (parent != nullptr)
	{
		toast->setFixedSize(parent->size());
        toast->show();
	}
    else
    {
        toast->show();
        QEventLoop loop;
        QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
        loop.exec();
        delete toast;
    }
}

#if 0
void Toast::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QWidget::paintEvent(ev);
}

void Toast::resizeEvent(QResizeEvent *ev)
{
	//m_label->setMinimumSize(256, 67);
	//m_label->move((size().width() - m_label->size().width()) / 2, (size().height() - m_label->size().height()) / 2);
}
#endif