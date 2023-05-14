#include "IPlayerCore.h"

#include <QMap>
#include <QMutex>

#include <atomic>

class QTimer;
class CPlayerCore : public IPlayerCore
{
    Q_OBJECT
    friend class CPlayerCoreManager;

public:
    static CPlayerCore *instance(const QString &url);
    virtual ~CPlayerCore() Q_DECL_OVERRIDE;
    bool isUsed() const {return m_use > 0;}
    void use();
    void unuse();


public slots:
    void reopen();
    void dopen();
    void openOutput(const QString& mp4RecordPath);
    void closeOutput(const QString& mp4RecordPath);


protected:
    void run() Q_DECL_OVERRIDE;

protected slots:
    void onTimerCounter();
    void playResolutionChanged();

signals:
    void remove(const QString &url);

private:
    explicit CPlayerCore(const QString &url);

    QString                 m_url;          // 播放路径
    int                     m_counter;      // 销毁计数
    std::atomic_int         m_use;          // 引用计数
	QString                 m_outputUrl;    // 输出url

    QTimer                  *m_timer = nullptr;
};

class CPlayerCoreManager : public QObject
{
    Q_OBJECT
public:
    ~CPlayerCoreManager() Q_DECL_OVERRIDE;
    static CPlayerCoreManager *instance();
    CPlayerCore *getCPlayerCore(const QString &url);

public slots:
    void remove(const QString &url);

private:
    explicit CPlayerCoreManager(QObject *parent = Q_NULLPTR);
    static      CPlayerCoreManager *s_instance;                
    static      QMutex s_mutex;                         // 单例锁
    typedef     QMap<QString, CPlayerCore*> DMap;
    DMap        m_map;                                  
    QMutex      m_mutex;                                // 数据锁
};

