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
    // ����������rtsp��ַ
    void setPlayRtspUrl(const QString &subRtspUrl);

    // ����¼����rtsp��ַ��������
    void setSaveRtspUrl(const QString &url);

    void setWaitStr(const QString &str);
    void setWaitPic(const QPixmap& pix);

    enum ResizeType {
        Default = 0,    ///< �����ֱ���
        ByWidth,        ///< ���ݿ�ȼ���߶�
        ByHeight        ///< ���ݸ߶ȼ�����
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
    // ��������
    void openSubStream();
    // ��������
    void openMainStream();

    void startRecordSig(const QString& mp4RecordPath);
    void stopRecordSig(const QString& mp4RecordPath);

private:
    QString       m_playRtspUrl;               // ����·����������rtsp��ַ
    QString       m_saveRtspUrl;               // ¼����������rtsp��ַ
    QString       m_savePath;                  // ¼���ļ�·��
    CPlayerCore*  m_CPlayerCore    = nullptr;  // ������������Ƶ��
    CPlayerCore*  m_savePlayerCore = nullptr;  // ¼����������Ƶ��
    RenderThread* m_render         = nullptr;  // ��Ⱦ�̣߳���������
    bool          m_isPause        = false;    // �Ƿ���ͣ
    bool          m_isRecord       = false;    // �Ƿ�����¼��
    bool          m_isWait         = true;     // �Ƿ��ڵȴ�״̬
    bool          m_hwDecode       = false;    // �Ƿ�ʹ��Dxva������Ⱦ
    QString       m_waitStr;                   // �ȴ�����ʾ�ַ���
    QPixmap       m_waitPic;
    ResizeType    m_resizeType;   // ���ڴ�С�仯����
    qreal         m_aspectRatio;  // ���ڱ���
    QString       m_playErrorTips;
    QString       m_saveErrorTips;
    QString       m_errorTips;
    bool          m_canShowErrorTips = true;
};

