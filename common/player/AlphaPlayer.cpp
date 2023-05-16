#include "AlphaPlayer.h"
#include "IPlayerCore.h"
#include <GlobalConfig.h>
#include <util.h>

#include <QEvent>
#include <QPainter>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include <QAudioOutput>
#include <QDir>
#include <QSize>
#include <QResizeEvent>

AlphaPlayer::AlphaPlayer(QWidget *parent)
    : QLabel(parent)
    , m_playerCore(Q_NULLPTR)
    , m_isPause(false)
    , m_resizeType(ResizeType::Default)
    , m_aspectRatio(0)
    , m_isRecord(false)
    , m_isWait(true)
{
    // 有英伟达显卡的时候则大屏使用DXVA解码渲染
    if (0 && util::haveNvidiaGpu())
    {
        m_hwDecode = true;
        // 大屏用Dxva渲染，所以不传this
        m_playerCore = new IPlayerCore(/*this*/);
        m_playerCore->enableHwDecode();
        m_playerCore->setWindowHandle((HWND)this->winId());
    }
    else
    {
        m_hwDecode = false;
        m_playerCore = new IPlayerCore(this);
    }

    connect(this, &AlphaPlayer::doOpen, m_playerCore, &IPlayerCore::open);
    connect(this, &AlphaPlayer::doClose, m_playerCore, &IPlayerCore::close);
    connect(m_playerCore, &IPlayerCore::begin, this, &AlphaPlayer::onXBegin);
    connect(m_playerCore, &IPlayerCore::draw, this, &AlphaPlayer::draw);
}

AlphaPlayer::~AlphaPlayer()
{
    stop();
}

bool AlphaPlayer::isOpened() const
{
    return m_playerCore->isOpened();
}

void AlphaPlayer::setPath(const QString &path)
{
    if (path.isEmpty() || path == m_path)
    {
        return;
    }
	if (!isOpened())
	{
		m_path = path;
	}
}

void AlphaPlayer::setWaitStr(const QString &str)
{
    m_waitStr = str;
    //m_waitPic = "";
    reset();
}

void AlphaPlayer::setWaitPic(const QPixmap& pix)
{
    //m_waitPic = url;
    m_waitPic = pix;
    m_waitStr = "";
    reset();
}

void AlphaPlayer::setResizeType(AlphaPlayer::ResizeType resizeType, qreal aspectRatio)
{
    m_resizeType = resizeType;
    m_aspectRatio = aspectRatio;
}

void AlphaPlayer::resizeEvent(QResizeEvent *event)
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

void AlphaPlayer::play()
{
	if (!m_path.isEmpty())
	{
		if (!m_doOpenCalled)
		{
			m_doOpenCalled = true;
			emit doOpen(m_path);
		}
	}
}

void AlphaPlayer::pause(bool isPause)
{
    m_playerCore->pause(isPause);
}

void AlphaPlayer::stop()
{
	if (m_doOpenCalled)
	{
		m_doOpenCalled = false;
		reset();
		emit doClose();
	}
    if (m_hwDecode)
    {
        setUpdatesEnabled(true);
    }
    m_path = "";
}

void AlphaPlayer::addOutput(const QString &url)
{
    if (url.isEmpty()) return;
    removeOutput();
    m_isRecord = true;
    m_file = url;
    m_playerCore->addOutput(m_file);
}

void AlphaPlayer::removeOutput()
{
    if (!m_isRecord) return;
    m_isRecord = false;
    if (m_file.isEmpty()) return;
    m_playerCore->rmOutput(m_file);
}

void AlphaPlayer::reset()
{
    m_isWait = true;
    if (!m_waitPic.isNull())
    {
        QPixmap pix(m_waitPic);
        if (!pix.isNull())
        {
            pix = pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            setPixmap(pix);
        }
    }
    else if (!m_waitStr.isEmpty())
    {
        setText(m_waitStr);
    }
}

void AlphaPlayer::draw(QImage img, const QString &url)
{
    if (url != m_path) return;
    if (m_isWait) return;

    setPixmap(QPixmap::fromImage(img));
}

void AlphaPlayer::onXBegin(void *)
{
    m_isWait = false;
    if (m_hwDecode)
    {
        setUpdatesEnabled(false);
    }
}
