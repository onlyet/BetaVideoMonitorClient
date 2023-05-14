
#include <QLabel>

class IPlayerCore;

class AlphaPlayer : public QLabel
{
    Q_OBJECT
public:

    explicit AlphaPlayer(QWidget *parent = nullptr);
    virtual ~AlphaPlayer() Q_DECL_OVERRIDE;

    bool isOpened() const;
    bool isPause() const {return m_isPause;}
    bool isRecording() const {return m_isRecord;}
    const QString &path() const {return m_path;}
    void setPath(const QString &path);
    void setWaitStr(const QString &str);
    void setWaitPic(const QPixmap& pix);

    enum ResizeType{
        Default = 0,    ///< 不保持比例
        ByWidth,        ///< 根据宽度计算高度
        ByHeight        ///< 根据高度计算宽度
    };

    void setResizeType(ResizeType resizeType, qreal aspectRatio = 0);

public slots:
    void play();
    void pause(bool isPause);
    void stop();
    void addOutput(const QString &url);
    void removeOutput();

protected:
    virtual void reset();
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void draw(QImage img, const QString &url);
    void onXBegin(void *ptr);

signals:
    void doOpen(const QString &path);
    void doClose();

private:
    IPlayerCore     *m_playerCore;          
    QString         m_path;                 // 播放路径
    bool            m_isPause;              // 是否暂停

    QString         m_waitStr;              // 等待的显示字符串
    QPixmap         m_waitPic;

    ResizeType      m_resizeType;           // 窗口大小变化类型
    qreal           m_aspectRatio;          // 窗口比例

    QString         m_file;                 // 录像保存路径
    bool            m_isRecord;             // 是否正在录像

    bool            m_isWait;               // 是否处于等待状态

	bool            m_doOpenCalled = false;

    bool            m_hwDecode = true;      // 是否用Dxva解码渲染
};

