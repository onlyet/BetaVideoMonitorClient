#include "CPlayerCore.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <util.h>

/*! @def 延时回收计数 */
#define DESTROY_COUNTER 0

CPlayerCore::CPlayerCore(const QString &url)
    : IPlayerCore(Q_NULLPTR)
    , m_url(url)
    , m_counter(0)
    , m_use(0)
{
    connect(this, &IPlayerCore::resolutionChanged, this, &CPlayerCore::playResolutionChanged);
}

// 在主线程析构
CPlayerCore::~CPlayerCore()
{
    //qDebug() << "~CPlayerCore 1" << m_url << m_outputUrl;
    quit();
    wait();
    //qDebug() << "~CPlayerCore() 2" << m_url << m_outputUrl;
}

void CPlayerCore::use()
{
    ++m_use;
    //qDebug() << QString("%1 m_use: %2").arg(util::logRtspUrl(m_url)).arg(m_use);
}

void CPlayerCore::unuse()
{
    --m_use;
    //qDebug() << QString("%1 m_use: %2").arg(util::logRtspUrl(m_url)).arg(m_use);
}

CPlayerCore *CPlayerCore::instance(const QString &url)
{
    // 在主线程创建
    return CPlayerCoreManager::instance()->getCPlayerCore(url);
}

void CPlayerCore::reopen()
{
    qInfo() << qstr("重新打开%1").arg(util::logRtspUrl(m_url));
    open(m_url);
}

void CPlayerCore::dopen()
{
    QWidget *wid = qobject_cast<QWidget*>(sender());
    if (!wid)
    {
        return;
    }

    if (isOpened())
    {
        emit begin(sender());
    }
    else
    {
        open(m_url);
    }
}

void CPlayerCore::openOutput(const QString& mp4RecordPath) {
    if (!isOpened()) {
        QMetaObject::Connection* const connection = new QMetaObject::Connection;
        *connection                               = connect(this, &IPlayerCore::inputOpened, [this, connection, mp4RecordPath]() {
            QObject::disconnect(*connection);
            delete connection;
            use();
            addOutput(mp4RecordPath);
        });
    } else {
        use();
        addOutput(mp4RecordPath);
    }
}

void CPlayerCore::closeOutput(const QString& mp4RecordPath) {
    if (isOpened())
    {
        rmOutput(mp4RecordPath);
        unuse();
    }
}

void CPlayerCore::playResolutionChanged()
{
    reopen();
}

void CPlayerCore::run()
{
    m_counter = 0;
    m_timer = new QTimer;
    connect(m_timer, &QTimer::timeout, this, &CPlayerCore::onTimerCounter);
    m_timer->start(100);
    exec();
    delete m_timer;
    m_timer = Q_NULLPTR;
}

void CPlayerCore::onTimerCounter()
{
    if(isUsed())
    {
        m_counter = 0;
        return;
    }

    ++m_counter;
    if (m_counter >= DESTROY_COUNTER)
    {
        // 避免再次触发本函数
        m_timer->stop();
        doClose();
        m_counter = 0;
        emit remove(path());
    }
}

CPlayerCoreManager::CPlayerCoreManager(QObject *parent)
    : QObject(parent)
{

}

CPlayerCoreManager::~CPlayerCoreManager()
{
    qDeleteAll(m_map);
    m_map.clear();
}

CPlayerCoreManager *CPlayerCoreManager::s_instance = Q_NULLPTR;
QMutex CPlayerCoreManager::s_mutex;
CPlayerCoreManager *CPlayerCoreManager::instance()
{
    if(!s_instance)
    {
        s_mutex.lock();
        if(!s_instance)
        {
            s_instance = new CPlayerCoreManager(qApp);
        }
        s_mutex.unlock();
    }
    return s_instance;
}

CPlayerCore* CPlayerCoreManager::getCPlayerCore(const QString& url) {
    if (!m_map.contains(url))
    {
        m_mutex.lock();
        if (!m_map.contains(url))
        {
            CPlayerCore *dff = new CPlayerCore(url);
            connect(dff, &CPlayerCore::remove, this, &CPlayerCoreManager::remove);
            m_map.insert(url, dff);
            dff->start();
        }
        m_mutex.unlock();
    }
    return m_map[url];
}

void CPlayerCoreManager::remove(const QString &url)
{
    if (m_map.contains(url))
    {
        m_mutex.lock();
        if (m_map.contains(url))
        {
            if (!m_map[url]->isUsed())
            {
                delete m_map[url];
                m_map.remove(url);
            }
        }
        m_mutex.unlock();
    }
}
