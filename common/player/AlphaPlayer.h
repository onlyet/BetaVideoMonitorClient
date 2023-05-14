
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
        Default = 0,    ///< �����ֱ���
        ByWidth,        ///< ���ݿ�ȼ���߶�
        ByHeight        ///< ���ݸ߶ȼ�����
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
    QString         m_path;                 // ����·��
    bool            m_isPause;              // �Ƿ���ͣ

    QString         m_waitStr;              // �ȴ�����ʾ�ַ���
    QPixmap         m_waitPic;

    ResizeType      m_resizeType;           // ���ڴ�С�仯����
    qreal           m_aspectRatio;          // ���ڱ���

    QString         m_file;                 // ¼�񱣴�·��
    bool            m_isRecord;             // �Ƿ�����¼��

    bool            m_isWait;               // �Ƿ��ڵȴ�״̬

	bool            m_doOpenCalled = false;

    bool            m_hwDecode = true;      // �Ƿ���Dxva������Ⱦ
};

