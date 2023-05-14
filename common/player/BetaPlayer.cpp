#include "BetaPlayer.h"
#include "CPlayerCore.h"
#include <GlobalConfig.h>
#include <util.h>
#include <GlobalSignalSlot.h>

#include <QPainter>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include <QAudioOutput>
#include <QDir>
#include <QSize>
#include <QEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QToolTip>
#include <QMouseEvent>
#include <QPainter>

BetaPlayer::BetaPlayer(QWidget *parent)
    : QLabel(parent)
    , m_resizeType(ResizeType::Default)
    , m_aspectRatio(0)
{
    setScaledContents(true);
}

BetaPlayer::~BetaPlayer()
{
    //qDebug() << "~BetaPlayer() 1";
    stop();
    //qDebug() << "~BetaPlayer() 2";
}

bool BetaPlayer::isOpened() const
{
    return m_CPlayerCore && m_CPlayerCore->isOpened();
}

void BetaPlayer::setPlayRtspUrl(const QString &subRtspUrl)
{
    m_playRtspUrl = subRtspUrl;
}

void BetaPlayer::setSaveRtspUrl(const QString &url)
{
    m_saveRtspUrl = url;
}

void BetaPlayer::setWaitStr(const QString &str)
{
    m_waitStr = str;
    //m_waitPic = "";
    reset();
}

void BetaPlayer::setWaitPic(const QPixmap& pix)
{
    //m_waitPic = url;
    m_waitPic = pix;
    m_waitStr = "";
    reset();
}

void BetaPlayer::play() {
    if (m_playRtspUrl.isEmpty()) {
        return;
    }

    if (!m_CPlayerCore) {
        m_CPlayerCore = CPlayerCore::instance(m_playRtspUrl);
        m_CPlayerCore->use();
        connect(m_CPlayerCore, &IPlayerCore::begin, this, &BetaPlayer::onXBegin);
        connect(m_CPlayerCore, &IPlayerCore::IpcParamsError, this, &BetaPlayer::setPlayErrorTips);
        connect(this, &BetaPlayer::openSubStream, m_CPlayerCore, &CPlayerCore::dopen);
        //connect(this, &BetaPlayer::reopenSubStream, m_CPlayerCore, &CPlayerCore::reopen);

        if (m_hwDecode) {
            m_CPlayerCore->enableHwDecode();
        } else {
            if (m_render) {
                m_render->setFFmpeg(m_CPlayerCore);
                if (!m_render->isRunning()) {
                    m_render->start();
                }
            }
        }

        // 小屏播放需要解码
        m_CPlayerCore->enableDecode(true);
        m_CPlayerCore->setWindowHandle((HWND)this->winId());
    }
    emit openSubStream();
}

void BetaPlayer::stop()
{
    //QTime t9 = QTime::currentTime();
    if (m_render)
    {
        m_render->setFFmpeg(nullptr);
    }
    //qDebug() << "t9 time:" << t9.elapsed();

    if (!m_CPlayerCore)
    {
        return;
    }

    disconnect(m_CPlayerCore);
    m_CPlayerCore->disconnect(this);
    m_CPlayerCore->unuse();
    m_CPlayerCore = nullptr;

    reset();
}

void BetaPlayer::addOutput(const QString& mp4Path) {
    if (mp4Path.isEmpty())
    {
        qDebug() << "savePath.isEmpty";
        return;
    }
    if (m_saveRtspUrl.isEmpty())
    {
        qDebug() << "m_saveRtspUrl.isEmpty";
        return;
    }
    if (m_isRecord)
    {
        return;
    }
    m_isRecord = true;

    // 打开主码流rtsp地址
    if (!m_savePlayerCore)
    {
        // 单码流，比如某USB版本
        if (m_saveRtspUrl == m_playRtspUrl)
        {
            m_savePlayerCore = m_CPlayerCore;
            m_savePlayerCore->use();
        }
        // 双码流
        else
        {
            m_savePlayerCore = CPlayerCore::instance(m_saveRtspUrl);
            m_savePlayerCore->use();
            connect(m_savePlayerCore, &IPlayerCore::IpcParamsError, this, &BetaPlayer::setSaveErrorTips);
            connect(m_savePlayerCore, &IPlayerCore::concatStarted, &GSignalSlot, &GlobalSignalSlot::concatStarted);
            connect(m_savePlayerCore, &IPlayerCore::concatDone, &GSignalSlot, &GlobalSignalSlot::concatDone);
            connect(this, &BetaPlayer::openMainStream, m_savePlayerCore, &CPlayerCore::dopen);
            connect(this, &BetaPlayer::startRecordSig, m_savePlayerCore, &CPlayerCore::openOutput);
            connect(this, &BetaPlayer::stopRecordSig, m_savePlayerCore, &CPlayerCore::closeOutput);
            
            // 主码流不解码
            m_savePlayerCore->enableDecode(false);

            emit openMainStream();
        }
    }

    m_savePath = mp4Path;
    emit startRecordSig(m_savePath);
}

void BetaPlayer::removeOutput()
{
    if (!m_savePlayerCore)
    {
        return;
    }
    if (m_savePath.isEmpty())
    {
        return;
    }
    if (!m_isRecord)
    {
        return;
    }
    m_isRecord = false;

    m_savePlayerCore->unuse();
    m_savePlayerCore = nullptr;
    emit stopRecordSig(m_savePath);
}

void BetaPlayer::pause(bool isPause)
{
    m_isPause = isPause;
    if (m_CPlayerCore)
    {
        // 暂停则不解码
        m_CPlayerCore->enableDecode(!m_isPause);
    }

    if (!m_hwDecode) {
        if (m_render) {
            m_render->pause(m_isPause);
        }
    }

    setPlayerWait(isPause);
}

void BetaPlayer::enableHwDecode(bool enabled)
{
    m_hwDecode = enabled;
    if (m_hwDecode)
    {
    }
    else
    {
        if (!m_render)
        {
            m_render = new RenderThread(this);
            connect(m_render, &RenderThread::draw, this, &BetaPlayer::draw);
            m_render->setDstWinHandle((int)this->winId());
        }
    }
}

void BetaPlayer::reset()
{
    m_isWait = true;
    if (!m_waitPic./*isEmpty*/isNull())
    {
        if (m_hwDecode)
        {
            QTimer::singleShot(1000, [this]() {
                QPixmap pix(m_waitPic);
                if (!pix.isNull())
                {
                    pix = pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    setPixmap(pix);
                }
                setUpdatesEnabled(true);
            });
            QTimer::singleShot(2000, [this]() {
                update();
            });
        }
        else
        {
            QPixmap pix(m_waitPic);
            if (!pix.isNull())
            {
                //pix = pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                setPixmap(pix);
                setScaledContents(true);
            }
        }
    }
    else
    {
        if (m_hwDecode)
        {
            QTimer::singleShot(1000, [this]() {
                setText(m_waitStr);
                setUpdatesEnabled(true);
            });
            QTimer::singleShot(2000, [this]() {
                update();
            });
        }
        else
        {
            setText(m_waitStr);
        }
    }
}

// 显示错误提示
bool BetaPlayer::event(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (!m_errorTips.isEmpty()) {
            QRect rect(this->width() - 20, 0, 20, 20);
            if (rect.contains(helpEvent->pos())) {
                QToolTip::showText(helpEvent->globalPos(), m_errorTips);
                return true;
            }
        }
        QToolTip::hideText();
        event->ignore();
        return true;
    }
    return QWidget::event(event);
}

//void BetaPlayer::paintEvent(QPaintEvent *e)
//{
//    QPainter painter(this);
//    painter.drawLine(0, 0, 20, 20);
//}

void BetaPlayer::setResizeType(BetaPlayer::ResizeType resizeType, qreal aspectRatio)
{
    m_resizeType = resizeType;
    m_aspectRatio = aspectRatio;
}

void BetaPlayer::setPlayerWait(bool wait)
{
    if (wait)
    {
        m_isWait = true;
        if (!m_waitPic.isNull())
        {
            QPixmap pix(m_waitPic);
            if (!pix.isNull())
            {
                //pix = pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                setPixmap(pix);
            }
        }
        else
        {
            setText(/*m_waitStr*/ "pause");
        }
    }
    else
    {
        m_isWait = false;
    }

}

void BetaPlayer::resizeEvent(QResizeEvent *event)
{
    if(ResizeType::Default == m_resizeType || m_aspectRatio <= 0)
    {
        return QLabel::resizeEvent(event);
    }

    QSize mSize = event->size();
    if(ResizeType::ByWidth == m_resizeType)
    {
        int h = mSize.width() / m_aspectRatio;
        setMinimumHeight(h);
        return;
    }

    if(ResizeType::ByHeight == m_resizeType)
    {
        int w = mSize.height() * m_aspectRatio;
        setMinimumWidth(w);
        return;
    }
}

void BetaPlayer::draw(QImage img, const QString &url)
{
    if(isHidden()) return;
    if(m_isPause) return;
    if (url != m_playRtspUrl) return;
    if (m_isWait) return;

    // FIXME: 这里为什么会崩溃？
    QPixmap pix = QPixmap::fromImage(img);

    if (!m_errorTips.isEmpty())
    {
        QPainter painter(&pix);
        painter.fillRect(pix.width() - 20, 0, 20, 20, QBrush(QColor(255, 0, 0)));
    }

    setPixmap(pix);
}

void BetaPlayer::onXBegin(void *ptr)
{
    if (ptr && ptr != this)
    {
        return;
    }
    if (!m_CPlayerCore)
    {
        return;
    }
    m_isWait = false;

    if (m_hwDecode)
    {
#if 1
        setUpdatesEnabled(false);
#endif
    }
}

void BetaPlayer::setPlayErrorTips(const QString &tips)
{
    if (m_canShowErrorTips)
    {
        m_playErrorTips = tips;
        setErrorTips();
    }
}

void BetaPlayer::setSaveErrorTips(const QString & tips)
{
    // 单码流，比如某USB版本
    if (m_saveRtspUrl == m_playRtspUrl)
    {
        // TODO
        return;
    }

    m_saveErrorTips = tips;
    setErrorTips();
}

void BetaPlayer::setErrorTips()
{
    m_errorTips = "";
    if (!m_playErrorTips.isEmpty())
    {
        //m_errorTips.append(qstr("%1\n\n").arg(m_playErrorTips));
        m_errorTips = m_playErrorTips;
    }
    if (!m_saveErrorTips.isEmpty())
    {
        if (!m_playErrorTips.isEmpty())
        {
            m_errorTips.append("\n\n");
        }
        m_errorTips.append(m_saveErrorTips);
    }
    if (m_errorTips.isEmpty())
    {
        return;
    }

    // 显示红点
    QPixmap pix(size());
    pix.fill(QColor(83, 83, 83));
    QPainter painter(&pix);
    painter.fillRect(pix.width() - 20, 0, 20, 20, QBrush(QColor(255, 0, 0)));
    QPixmap logo(m_waitPic);
    if (!logo.isNull())
    {
        logo = logo.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    painter.drawPixmap((width() - logo.width()) / 2, (height() - logo.height()) / 2, logo);
    setPixmap(pix);
}

void BetaPlayer::enableSetErrorTips(bool enabled)
{
    m_canShowErrorTips = enabled;
}
