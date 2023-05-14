#include "CCenter.h"
#include "CSmallScreen.h"
#include <util.h>
#include <GlobalConfig.h>
#include <CTransparentLabel.h>

#include <QScrollBar>
#include <QWheelEvent>
#include <QTcpSocket>
#include <QTimer>
#include <QStyleOption>
#include <QPainter>
#include <QTime>

#include <functional>

CCenter::CCenter(QWidget *parent) :
    IFlowWidget(parent)
{
    m_scrollStep = 60;

    setStyleSheet("CCenter{border:none}"
                  "QScrollBar:vertical { background-color: #F9F9F9; width: 21px; margin: 0px 7px 0px 7px; border: 1px solid gray;}"
                  "QScrollBar::handle:vertical { background-color: #337BEC; min-height: 9px; border-radius: 4px; margin: -1px -2px -1px -2px}"
                  "QScrollBar:horizontal { background-color: #F9F9F9; height: 21px; margin: 7px 0px 7px 0px ; border: 1px solid gray }"
                  "QScrollBar::handle:horizontal { background-color: #337BEC; min-width: 9px; border-radius: 4px; margin: -2px -1px -2px -1px }"
                  "QScrollBar::sub-line, QScrollBar::add-line { background:none }"
                  "QScrollBar::up-arrow, QScrollBar::down-arrow { background:none }"
                  "QScrollBar::add-page, QScrollBar::sub-page { background:none }");

    m_maxSeatNum = 28;
    createSeat();
}

CCenter::~CCenter()
{
    clearUI();
    qDeleteAll(m_seatWidgetList);
    m_seatWidgetList.clear();
}

void CCenter::createSeat()
{
    for (int i = 0; i < m_maxSeatNum; ++i)
    {
        CSmallScreen *seat = new CSmallScreen();
        int serialNum = i + 1;
        seat->setSerialNum(serialNum);

        if (!CONFIG.enableServer())
        {
            seat->setSeatInfo(serialNum);
        }

        connect(seat, &CSmallScreen::focus, this, &CCenter::onFocus);
        m_seatWidgetList.append(seat);
    }
}

void CCenter::init1PageSeat()
{
    if (m_1PageSeatNum > m_seatWidgetList.length())
    {
        qDebug() << "logic error";
        return;
    }

    QList<QWidget *> items;
    for (int i = 0; i < m_1PageSeatNum; ++i)
    {
        items.append(m_seatWidgetList[i]);
    }
    addItems(items);
}

void CCenter::init1PageSeat(int min, int max)
{
    if (m_1PageSeatNum > m_seatWidgetList.length())
    {
        qDebug() << "logic error";
        return;
    }
    QList<QWidget *> items;
    for (int i = 0; i < m_maxSeatNum; ++i)
    {
        if (i < min || i > max)
        {
            continue;
        }
        items.append(m_seatWidgetList[i]);
    }
    addItems(items);
}

void CCenter::init()
{
    int hSpace = 6;
    int vSpace;
    int hMargin = 6;
    int vMargin;
    switch (m_1PageSeatNum)
    {
    case 24:
    case 28:
        vSpace = 6;
        vMargin = 6;
        break;
    case 12:
        vSpace = 60;
        vMargin = 60;
        break;
    case 6:
    case 8:
        vSpace = 120;
        vMargin = 120;
        break;
    default:
        vSpace = 6;
        vMargin = 6;
        break;
    }

    vSpace = util::scaleWidthByResolution(vSpace);
    vMargin = util::scaleWidthByResolution(vMargin);
    QSize cs = CSmallScreen::getPerferSize(size(), m_row, m_column, hSpace, vSpace, hMargin, vMargin);

    QVariantMap spaceInfo;
    spaceInfo.insert("hSpace", hSpace);
    spaceInfo.insert("vSpace", vSpace);
    spaceInfo.insert("hMargin", hMargin);
    spaceInfo.insert("vMargin", vMargin);
    spaceInfo.insert("aPageSeatNum", m_1PageSeatNum);

    // 显示一页的数量
    int itemCnt = count();
    for(int i = 0; i < itemCnt; ++i)
    {
        CSmallScreen *seat = qobject_cast<CSmallScreen*>(item(i));
        if (!seat)
        {
            continue;
        }
        seat->setSize(cs, spaceInfo);
    }

    reset(spaceInfo); 

}

void CCenter::loadData()
{
    int n = m_maxSeatNum;
    for(int i = 0; i < n; ++i)
    {
        int serialNum = i + 1;
        CSmallScreen *seat = m_seatWidgetList[i];
        seat->setSeatInfo(serialNum);

        if (1)
        {
            QString leftUrl  = "";
            QString rightUrl = "";
            seat->setLeftUrl(leftUrl);
            seat->setRightUrl(rightUrl);
            qDebug() << qstr("seat: %1, name: %2, %3, %4")
                .arg(seat->seat()).arg(seat->name()).arg(util::logRtspUrl(leftUrl)).arg(util::logRtspUrl(rightUrl));
        }
    }
}

void CCenter::pause(bool isPause)
{
    for(int i = 0; i < m_maxSeatNum; ++i)
    {
        CSmallScreen *seat = m_seatWidgetList[i];

        seat->pause(isPause);
    }
}

void CCenter::openAll() {
    for (int i = 0; i < m_maxSeatNum; ++i) {
        CSmallScreen *seat = m_seatWidgetList[i];
        if (!seat) continue;

        if (seat->name().isEmpty()) {
            continue;
        }
        seat->startPlay();
    }
}

void CCenter::closeAll()
{
    for (int i = 0; i < m_maxSeatNum; ++i)
    {
        CSmallScreen* seat = m_seatWidgetList[i];
        if (!seat) continue;

        if (!seat || seat->name().isEmpty())
        {
            continue;
        }
        seat->stopPlay();
    }
}

void CCenter::allPeopleStart()
{
    qInfo() << qstr("所有座位开始播放和录制");
    QTime t = QTime::currentTime();
    if (CONFIG.enableServer())
    {
        int size = 28;
        if (size > m_maxSeatNum)
        {
            qDebug() << "logic error";
            return;
        }
        for (int i = 0; i < size; ++i)
        {
            CSmallScreen *pSeat = m_seatWidgetList[i];
            if (!pSeat) continue;

            pSeat->startRecord();
        }
    }
    else
    {
        // FIXME
        int n = m_maxSeatNum;
        //int n = 1;
        for (int i = 0; i < n; ++i)
        {
            CSmallScreen *tmpSeat = m_seatWidgetList[i];
            if (!tmpSeat) continue;

            tmpSeat->startPlay();
            tmpSeat->startRecord();
        }
    }
    //qDebug() << "allPeopleStart time:" << t.elapsed();
}

void CCenter::allPeopleStop()
{
    qInfo() << qstr("所有座位结束播放和录制");
    if (CONFIG.enableServer())
    {
        int num = 28;
        for (int i = 0; i < num; ++i)
        {
            CSmallScreen *seat = m_seatWidgetList[i];
            if (!seat) continue;

            seat->stopRecord();
            seat->stopPlay(); // 耗时

        }
    }
    else
    {
        // FIXME
        int n = m_maxSeatNum;
        for (int i = 0; i < n; ++i)
        {
            CSmallScreen *tmpSeat = m_seatWidgetList[i];
            if (!tmpSeat) continue;

            tmpSeat->stopRecord();
            tmpSeat->stopPlay();
        }
    }
}

void CCenter::setTransparentLabelVisable(bool visable, int delayMs)
{
    std::function<void()> f = [this, visable]() {
        for (int i = 0; i < count(); ++i)
        {
            CSmallScreen *seat = qobject_cast<CSmallScreen *>(item(i));
            if (!seat)
            {
                continue;
            }
            seat->setTransparentLabelVisable(visable);
        }
    };

    if (0 == delayMs)
    {
        f();
    }
    else
    {
        QTimer::singleShot(delayMs, [this, f]() {
            f();
        });
    }
}

void CCenter::switch24Screen()
{
    m_row = 6;
    m_1PageSeatNum = 24;

    setTransparentLabelVisable(false);

    clearUI();
    init1PageSeat();
    setRow(m_row);
    init();
    setTransparentLabelVisable(true, 1);
}

void CCenter::switch28Screen()
{
    m_row = 7;
    m_1PageSeatNum = 28;

    setTransparentLabelVisable(false);
    clearUI();
    init1PageSeat();
    setRow(m_row);
    init();
    setTransparentLabelVisable(true, 1);
}

void CCenter::switch12Screen()
{
    m_row = 3;
    m_1PageSeatNum = 12;

    setTransparentLabelVisable(false);
    clearUI();
    init1PageSeat();
    setRow(m_row);
    init();
    setTransparentLabelVisable(true, 1);
}

void CCenter::switch8Screen()
{
    m_row = 2;
    m_1PageSeatNum = 8;

    setTransparentLabelVisable(false);
    clearUI();
    init1PageSeat();
    setRow(m_row);
    init();
    setTransparentLabelVisable(true, 1);
}

void CCenter::currentPageChanged(int page)
{
    int min = m_1PageSeatNum * (page - 1);
    int max = m_1PageSeatNum * page - 1;

    setTransparentLabelVisable(false);
    clearUI();
    init1PageSeat(min, max);
    init();
    setTransparentLabelVisable(true, 1);
}

void CCenter::onFocus(QVariantMap data)
{
    pause(true);
    emit focus(data);
}
