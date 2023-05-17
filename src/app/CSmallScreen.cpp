#include <limits>

#include "CSmallScreen.h"
#include "ui_CSmallScreen.h"
#include "util.h"
#include "CPlayerCore.h"
#include <GlobalConfig.h>
#include <CMessageBox.h>
#include <CTransparentLabel.h>
#include <GlobalConfig.h>
#include <common.h>

#include <QMenu>
#include <QDebug>
#include <QMouseEvent>
#include <QString>
#include <QAction>
#include <QScreen>
#include <QFileInfo>

#define ColumnNum               4   // CPage1一页有多少列
#define TransparentLabelHeight  20  // 透明label高度
#define TitleFrameHeight        60  // 顶部标题栏高度

CSmallScreen::CSmallScreen(QWidget *parent, bool isPlaceHolder) :
    QFrame(parent)
    , ui(new Ui::CSmallScreen)
{
    ui->setupUi(this);

    initUi();

#if 0
    ui->camera_left->setWaitPic(CONFIG.logoPixmap());
    ui->camera_right->setWaitPic(CONFIG.logoPixmap());
#endif

    QString tmpLeftUrl = util::getSetting("Url/leftUrl", "").toString();
    QString tmpRightUrl = util::getSetting("Url/rightUrl", "").toString();
    static int s_j = 0;
#if 1
    if (s_j < 24)
#endif
    {
        if (!tmpLeftUrl.isEmpty()) {
            setLeftUrl(tmpLeftUrl);
        }
        if (!tmpRightUrl.isEmpty()) {
            setRightUrl(tmpRightUrl);
        }
    }
    ++s_j;
}

CSmallScreen::~CSmallScreen()
{
    delete ui;
}

int CSmallScreen::serialNum() const
{
    return m_serialNum;
}

void CSmallScreen::setSerialNum(int serialNum)
{
    m_serialNum = serialNum;

    if (m_serialNum <= CONFIG.gpuSeatCnt())
    {
        ui->camera_left->enableHwDecode(true);
        ui->camera_right->enableHwDecode(true);
    }
    else
    {
        ui->camera_left->enableHwDecode(false);
        ui->camera_right->enableHwDecode(false);
    }
}

void CSmallScreen::setSeatInfo(int serialNum)
{
    if (CONFIG.enableServer())
    {
        m_seat = serialNum;
		if (m_seat == 0) {
            m_serialNumLabel->setText(QLatin1String("- -"));
		}
		else
		{
			m_serialNumLabel->setText(QString::number(m_seat));
		}

        setInfo("他过江", "还有谁");
    }
    else
    {
        m_seat = m_serialNum;
        setInfo(qstr("他过江"), qstr("还有谁"));
    }
}

void CSmallScreen::setInfo(const QString &name, const QString &title)
{
    if (name.isEmpty())
    {
        ui->infoLabel->setText("");
        return;
    }
    m_name = name;
    m_title = title;
    util::setTextWithEllipsis(ui->infoLabel, qstr("%1").arg(name));
}

void CSmallScreen::setTransparentLabelVisable(bool visable)
{
    if (m_serialNumLabel)
    {
        m_serialNumLabel->setVisible(visable);
    }
}

void CSmallScreen::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    startPlay();
}

void CSmallScreen::setLeftUrl(const QString& url) {
    if (url.isEmpty()) {
        m_leftPlayUrl = "";
        m_leftSaveUrl = "";
        return;
    }
    QString subUrl = url;

    if (url.startsWith("rtsp")) {
        int index = url.size() - 1;
        // 海康
        // 最后一位：1主码流，2子码流
        if (url.contains("Streaming/Channels")) {
            subUrl.replace(index, 1, '2');
        }
        // 大华
        // subtype: 0主码流，1子码流
        else {
            subUrl.replace(index, 1, '1');
        }
    }
    ui->camera_left->setPlayRtspUrl(subUrl);
    ui->camera_left->setSaveRtspUrl(url);

    m_leftPlayUrl = subUrl;
    m_leftSaveUrl = url;
}

void CSmallScreen::setRightUrl(const QString& url) {
    if (url.isEmpty()) {
        m_rightPlayUrl = "";
        m_rightSaveUrl = "";
        return;
    }
    QString subUrl = url;

    if (url.startsWith("rtsp")) {
        int index = url.size() - 1;
        // 海康
        // 最后一位：1主码流，2子码流
        if (url.contains("Streaming/Channels")) {
            subUrl.replace(index, 1, '2');
        }
        // 大华
        // subtype: 0主码流，1子码流
        else {
            subUrl.replace(index, 1, '1');
        }
    }
    ui->camera_right->setPlayRtspUrl(subUrl);
    ui->camera_right->setSaveRtspUrl(url);

    m_rightPlayUrl = subUrl;
    m_rightSaveUrl = url;
}

const QString &CSmallScreen::leftUrl() const
{
    return ui->camera_left->path();
}

const QString &CSmallScreen::rightUrl() const
{
    return ui->camera_right->path();
}

void CSmallScreen::startPlay()
{
    if (1)
    {
        if (!m_leftPlayUrl.isEmpty())
        {
            ui->camera_left->play();
            ui->camera_left->pause(false);

        }
        if (!m_rightPlayUrl.isEmpty())
        {
            ui->camera_right->play();
            ui->camera_right->pause(false);
        }
    }
}

void CSmallScreen::stopPlay()
{
    if (!m_leftPlayUrl.isEmpty())
    {
        ui->camera_left->pause(true);
        ui->camera_left->stop(); // 主要是m_pScaler->setFFmpeg耗时
    }
    if (!m_rightPlayUrl.isEmpty())
    {
        ui->camera_right->pause(true);
        ui->camera_right->stop();
    }
}

void CSmallScreen::pause(bool isPause) const
{
    if (!m_leftPlayUrl.isEmpty())
    {
        ui->camera_left->pause(isPause);
    }
    if (!m_rightPlayUrl.isEmpty())
    {
        ui->camera_right->pause(isPause);
    }
}

void CSmallScreen::startRecord() {
    if (!CONFIG.recordVideoEnabled()) {
        return;
    }

    if (m_leftSaveUrl.isEmpty() && m_rightSaveUrl.isEmpty()) {
        return;
    }

    QString path = CONFIG.recordPath();

    if (CONFIG.enableServer()) {
        if (m_name.isEmpty()) {
            return;
        }

        QString middlePath = "test";
        QString userId     = m_serialNum;
        m_leftSavePath     = LeftRecordPath(path, middlePath, userId);
        m_rightSavePath    = RightRecordPath(path, middlePath, userId);

    } else {
        if (m_name.isEmpty()) {
            m_name = "test";
        }

        static qint64 s_i = 1;
        if (s_i >= MAXLONGLONG) {
            s_i = 1;
            qInfo() << "s_i = LONG_LONG_MAX";
        }
        m_leftSavePath  = QString("%1/test/%2_left.mp4").arg(path).arg(s_i);
        m_rightSavePath = QString("%1/test/%2_right.mp4").arg(path).arg(s_i);
        ++s_i;

    }

    if (m_recordStarted) {
        return;
    }
    m_recordStarted = true;

    if (!m_leftSaveUrl.isEmpty()) {
        ui->camera_left->addOutput(m_leftSavePath);
    }
    if (!m_rightSaveUrl.isEmpty()) {
        ui->camera_right->addOutput(m_rightSavePath);
    }
}

void CSmallScreen::stopRecord()
{
    if (!CONFIG.recordVideoEnabled())
    {
        return;
    }

	if (m_leftSaveUrl.isEmpty() && m_rightSaveUrl.isEmpty())
	{
		return;
	}

    if (!m_recordStarted)
    {
        return;
    }
    m_recordStarted = false;

	if (!m_leftSaveUrl.isEmpty())
	{
		ui->camera_left->removeOutput();
	}
	if (!m_rightSaveUrl.isEmpty())
	{
		ui->camera_right->removeOutput();
	}
}

QSize CSmallScreen::getPerferSize(QSize widgetSize, int row, int column, int xSpace, int ySpace, int &hMargin, int vMargin)
{
    int xSpaceSum = (column - 1) * xSpace;
    int ySpaceSum = (row - 1) * ySpace;
    int itemWidthSum = widgetSize.width() - xSpaceSum - hMargin * 2;
    int itemWidth = itemWidthSum / column;
    if (itemWidthSum % column == 0)
    {
        // 水平空间不能占满，否则第4列会变成空白，故多留点hspace
        itemWidth -= 1;
    }
    int itemHeight = (widgetSize.height() - ySpaceSum - vMargin * 2) / row;
    return QSize(itemWidth, itemHeight);
}

void CSmallScreen::setSize(const QSize &s, const QVariantMap &spaceInfo)
{
    bool isPauseLeft = ui->camera_left->isPause();
    bool isPauseRight = ui->camera_right->isPause();
    if (!isPauseLeft)
    {
        ui->camera_left->pause(true);
    }
    if (!isPauseRight)
    {
        ui->camera_right->pause(true);
    }

    int aPageSeatNum = spaceInfo["aPageSeatNum"].toInt();
    int hMargin = spaceInfo["hMargin"].toInt();
    int vMargin = spaceInfo["vMargin"].toInt();
    int hSpace = spaceInfo["hSpace"].toInt();
    int vSpace = spaceInfo["vSpace"].toInt();

    switch (aPageSeatNum)
    {
    case 24:
    case 28:
        ui->infoFrame->setFixedHeight(20);
        ui->infoLabel->setStyleSheet("QLabel{font-size: 16px; \
            font-family:PingFang-SC-Bold,PingFang-SC; \
            color: rgb(240, 240, 240); \
            line-height: 22px; \
            }");
        ui->infoLabel->setAlignment(Qt::AlignHCenter);
        ui->infoFrame->setContentsMargins(6, 3, 0, 0);
        break;
    case 12:
        ui->infoFrame->setFixedHeight(40);
        ui->infoLabel->setStyleSheet("QLabel{font-size: 24px; \
            font-family:PingFang-SC-Bold,PingFang-SC; \
            color: rgb(240, 240, 240); \
            line-height: 22px; \
            }");
        ui->infoLabel->setAlignment(Qt::AlignLeft);
        ui->infoFrame->setContentsMargins(10, 10, 10, 0);
        break;
    case 6:
    case 8:
        ui->infoFrame->setFixedHeight(60);
        ui->infoLabel->setStyleSheet("QLabel{font-size: 26px; \
            font-family:PingFang-SC-Bold,PingFang-SC; \
            color: rgb(240, 240, 240); \
            line-height: 22px; \
            }");
        ui->infoLabel->setAlignment(Qt::AlignLeft);
        ui->infoFrame->setContentsMargins(10, 20, 10, 0);
        break;
    default:
        break;
    }

    setFixedSize(s);
    ui->cameraFrame->setFixedHeight(s.height() - ui->infoFrame->height());
    int w = s.width() / 2 /2 * 2;
    int h = ui->cameraFrame->height() / 2 * 2;
    ui->camera_left->setGeometry(0, 0, w, h);
    ui->camera_right->setGeometry(w, 0, w, h);

    ui->infoLabel->setMaximumWidth(width() - 6);
    if (!m_name.isEmpty()) {
    util::setTextWithEllipsis(ui->infoLabel, qstr("%1").arg(m_name));
    }

    int x, y;
    int num = ((m_serialNum - 1) % aPageSeatNum);
    // 第几行，第几列，从0开始
    int nRow    = num / ColumnNum;
    int nColumn = num % ColumnNum;
    x           = hMargin + nColumn * (width() + hSpace);
    y = ui->cameraFrame->height() * (nRow + 1) + (ui->infoFrame->height() + vSpace) * nRow +
        TitleFrameHeight - TransparentLabelHeight + vMargin;
    QPoint p(x, y);
    m_serialNumLabel->move(p);
    m_serialNumLabel->setPaintText(QString::number(m_serialNum));

    if (!isPauseLeft)
    {
        ui->camera_left->pause(false);
    }
    if (!isPauseRight)
    {
        ui->camera_right->pause(false);
    }
}

void CSmallScreen::contextMenuEvent(QContextMenuEvent *event)
{
    if (CONFIG.enableServer())
    {
        // 空座位不添加右键菜单
        if (m_name.isEmpty()) return;
    }

    static const QString s_color = CONFIG.themeColorValue();
    static const QString s_qss = QString("QMenu { \
        background-color:rgb(89,87,87); \
        border: 3px solid %1; \
        } \
        QMenu::item{ \
        font-size: 10pt; \
        color: rgb(225,225,225);\
        border: 3px solid rgb(60,60,60);\
        background-color:rgb(89,87,87); \
        padding:6px 20px 6px 20px;\
        margin:2px 2px;\
        }\
        QMenu::item:selected{ \
        background-color:%2;\
        }\
        QMenu::item:pressed{\
        border: 1px solid rgb(60,60,61); \
        background-color: rgb(220,80,6); \
        }")
        .arg(s_color).arg(s_color);

    QMenu menu;
    menu.setStyleSheet(s_qss);

    if (1)
    {
        menu.addAction(ui->act_focus);
        menu.addAction(ui->act_open);
        menu.addAction(ui->act_close);
    }

    menu.exec(event->globalPos());
}

void CSmallScreen::on_act_focus_triggered()
{
    QVariantMap data;
    data.insert("seat", m_seat);
    data.insert("serialNum", m_serialNum);
    emit focus(data);
}

void CSmallScreen::initUi()
{ 
    initTransparentLabel();
}

void CSmallScreen::initTransparentLabel()
{
    if (!m_serialNumLabel)
    {
        m_serialNumLabel = new CTransparentLabel(QString::number(m_serialNum), this);
        m_serialNumLabel->setFont(QFont("Microsoft YaHei", 16));

        // 设置point size，避免修改分辨率导致文本显示不全
        QFont f("PingFangSC-Regular, PingFang SC");
        f.setPixelSize(22);
        m_serialNumLabel->setFont(f);
        // 不能用setFixedSize
        m_serialNumLabel->resize(28, 20);
    }
}

void CSmallScreen::setTransparentLabel(bool visable)
{
    if (m_serialNumLabel)
    {
        m_serialNumLabel->setVisible(visable);
    }
}

void CSmallScreen::on_act_open_triggered()
{
    startPlay();
}

void CSmallScreen::on_act_close_triggered()
{
    stopPlay();
}

