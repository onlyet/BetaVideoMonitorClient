#include "CBigScreen.h"
#include "ui_CBigScreen.h"
#include "util.h"
#include <GlobalConfig.h>

#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>

CBigScreen::CBigScreen(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CBigScreen)
{
    ui->setupUi(this);
    qreal aspectRadio = qreal(16) / qreal(9);
    ui->camera_left->setResizeType(AlphaPlayer::ByWidth, aspectRadio);
    ui->camera_right->setResizeType(AlphaPlayer::ByWidth, aspectRadio);

    ui->camera_left->setWaitPic(CONFIG.logoPixmap());
    ui->camera_right->setWaitPic(CONFIG.logoPixmap());

}

CBigScreen::~CBigScreen()
{
    delete ui;
}

void CBigScreen::init(QVariantMap data)
{
    m_seat      = data["seat"].toInt();
    ui->label_seat->setText(QStringLiteral("%1.").arg(m_seat));
    setName("他过江");

    QString leftUrl;
    QString rightUrl;

    if (1) {
        leftUrl  = "";
        rightUrl = "";

        QString tmpLeftUrl;
        QString tmpRightUrl;
        tmpLeftUrl  = util::getSetting("Url/leftUrl", "").toString();
        tmpRightUrl = util::getSetting("Url/rightUrl", "").toString();

        if(!tmpLeftUrl.isEmpty())
        {
            leftUrl = tmpLeftUrl;
        }
        if (!tmpRightUrl.isEmpty())
        {
            rightUrl = tmpRightUrl;
        }
    }
    else
    {
        leftUrl  = "";
        rightUrl = "";
    }

    ui->camera_left->setPath(leftUrl);
    ui->camera_right->setPath(rightUrl);

}

void CBigScreen::setName(const QString &name)
{
    if (CONFIG.enableServer())
    {
        m_name = name;
        ui->label_name->setText(QStringLiteral("%1").arg(m_name));
    }
    else
    {
        ui->label_name->setText(QStringLiteral("他过江"));
    }
}

void CBigScreen::startPlay() const
{
    ui->camera_left->play();
    ui->camera_right->play();
}

void CBigScreen::stopPlay() const
{
    ui->camera_left->stop();
    ui->camera_right->stop();
}

void CBigScreen::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(ui->act_back);
    menu.exec(event->globalPos());
}

// 打开一次大屏触发了两次showEvent
void CBigScreen::showEvent(QShowEvent *)
{
    setFocus();         // 设置焦点，否则无法触发按键事件
}

void CBigScreen::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        on_act_back_triggered();
    }
}

void CBigScreen::on_act_back_triggered()
{
    stopPlay();
    emit back();
}

