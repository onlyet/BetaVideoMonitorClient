#include <QLabel>
#include <QPixmap>

class CPlayerCore;
class RenderThread;

class BetaPlayer : public QLabel
{
    Q_OBJECT
public:
    explicit BetaPlayer(QWidget *parent = nullptr);
    virtual ~BetaPlayer() Q_DECL_OVERRIDE;

    bool isOpened() const;
    bool isPause() const {return m_isPause;}
    bool isRecording() const {return m_isRecord;}
    const QString &path() const { return m_playRtspUrl; }
    // 设置子码流rtsp地址
    void setPlayRtspUrl(const QString &subRtspUrl);

    // 设置录制用rtsp地址，主码流
    void setSaveRtspUrl(const QString &url);

    void setWaitStr(const QString &str);
    void setWaitPic(const QPixmap& pix);

    enum ResizeType {
        Default = 0,    ///< 不保持比例
        ByWidth,        ///< 根据宽度计算高度
        ByHeight        ///< 根据高度计算宽度
    };

    void setResizeType(ResizeType resizeType, qreal aspectRatio = 0);
    void setPlayerWait(bool wait);

public slots:
    void play();
    void stop();
    void addOutput(const QString& mp4Path);
    void removeOutput();
    void pause(bool isPause);

    void enableHwDecode(bool enabled = false);

    void setPlayErrorTips(const QString &tips);
    void setSaveErrorTips(const QString &tips);
    void setErrorTips();

    void enableSetErrorTips(bool enabled);

protected:
    virtual void reset();

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    bool event(QEvent *ev) override;

protected slots:
    void draw(QImage img, const QString &url);
    void onXBegin(void *ptr);

signals:
    // 打开子码流
    void openSubStream();
    // 打开主码流
    void openMainStream();

    void startRecordSig(const QString& mp4RecordPath);
    void stopRecordSig(const QString& mp4RecordPath);

private:
    QString       m_playRtspUrl;               // 播放路径，子码流rtsp地址
    QString       m_saveRtspUrl;               // 录制用主码流rtsp地址
    QString       m_savePath;                  // 录像文件路径
    CPlayerCore*  m_CPlayerCore    = nullptr;  // 播放子码流视频用
    CPlayerCore*  m_savePlayerCore = nullptr;  // 录制主码流视频用
    RenderThread* m_render         = nullptr;  // 渲染线程（子码流）
    bool          m_isPause        = false;    // 是否暂停
    bool          m_isRecord       = false;    // 是否正在录像
    bool          m_isWait         = true;     // 是否处于等待状态
    bool          m_hwDecode       = false;    // 是否使用Dxva解码渲染
    QString       m_waitStr;                   // 等待的显示字符串
    QPixmap       m_waitPic;
    ResizeType    m_resizeType;   // 窗口大小变化类型
    qreal         m_aspectRatio;  // 窗口比例
    QString       m_playErrorTips;
    QString       m_saveErrorTips;
    QString       m_errorTips;
    bool          m_canShowErrorTips = true;
};

