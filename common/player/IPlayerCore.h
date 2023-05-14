#include <QObject>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QRunnable>
#include <QWidget>
#include <QMap>

#include <atomic>

extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
}

#include "D3DVidRender.h"

//#define Enable_D3dRender
//#define Enable_Hardcode

class DecodeThread;
class RenderThread;
class Outputer;

class InputStream;

class IPlayerCore : public QThread
{
    Q_OBJECT
public:
    explicit IPlayerCore(QWidget *wid = Q_NULLPTR);
    virtual ~IPlayerCore() Q_DECL_OVERRIDE;

    bool toRgb(uchar *out, int outWidth = 0, int outHeight = 0);
    int stepVideoOnly();
    QString path() const;
    bool isOpened() const;
    double getFps() const;
    bool isEof() const;
    QSize getSize() const;
    void addOutput(const QString &url);
    void rmOutput(const QString &url);

    bool initD3D_NV12(HWND hwnd, int img_width, int img_height);
    bool initD3D_YUVJ420P(HWND hwnd, int img_width, int img_height);

    void enableDecode(bool enabled);

	QString codecName() const;

    // ���ò���������Ⱦ�ؼ���QLabel�ľ����
    void setWindowHandle(HWND hwnd);

    // ʹ��DXVAӲ��
    void enableHwDecode();

public slots:
    void open(const QString& path);
    void pause(bool isPause);
    void close();

protected:
    bool doOpen(const QString& path);
    void doClose();
    void init();
    AVPacket read();
    int decode(const AVPacket *pkt);
    QString parseError(int errNum);

    // �޸ĺϲ���Ƶ��Ҫ�õ���txt
    void appendConcatFile(const QString &videoPath);
    void concatVideo(const QString& videoPath);

signals:
    void draw(QImage img, const QString &url);
    void begin(void *ptr);              // ֪ͨ�����ѿ�ʼ

	// �Ѵ�����
	void inputOpened();

    void IpcParamsError(const QString &msg);
    void setNoError();

    void resolutionChanged();

    void concatStarted();
    void concatDone();

private:
    QString             m_path;                         // ����URL
    DecodeThread       *m_decoder = nullptr;            // ��ȡ�����߳�
    RenderThread        *m_render = nullptr;            // ��Ⱦ�߳�
    AVFormatContext     *m_pAVFormatCtx = nullptr;      // ���װ��
    AVCodecContext      *m_vDecodeCtx = nullptr;        // ��Ƶ������������
    SwsContext          *m_swsCtx = nullptr;
    AVFrame             *m_pYuv = nullptr;              // ��������Ƶ֡����
    QMutex              m_mutex;                        // ������������߳�ʱ����ͬʱ��Ķ�дAVFormatContext��AVCodecContext
    QMutex              m_mutex_output;                 // ����б�������
    QMap<QString, Outputer*> m_outputs;                 // ����б�
    QString             m_err;                          // ������Ϣ
    bool                m_isEof;                        // �Ƿ����;
    double              m_fps;                          // ��Ƶ��֡��
    bool                m_isOpened;                     // �Ƿ��Ѵ�ý��
    int                 m_videoStream;                  // ��Ƶ������
    QSize               m_size;                         // ��ǰ��Ƶ�ߴ�
    AVPixelFormat       m_pixFmt;                       // ��ǰ��Ƶ��ʽ
    bool                m_enableDecode = true;          // �Ƿ���룬�����������룬�򿪴�����С��������
	QString			    m_codecName;			        // ����������

    int                 m_codecCtxWidth = 0;
    int                 m_codecCtxHeight = 0;
    bool                m_resolutionChanged = false;
    std::atomic_bool    m_destructed = false;

#ifdef Enable_D3dRender
    CD3DVidRender	    m_d3d;
    int                 m_yuvSize;
    BYTE                *m_yuvBuf = nullptr;
#endif

    // dxva��Ⱦ���
public:
    HWND                    m_windowHandle;
    D3DPRESENT_PARAMETERS   m_d3dpp = { 0 };
    IDirect3DSurface9       *m_pBackBuffer = NULL;
    InputStream             *m_inputStream = nullptr;
    bool                    m_hwDecode = false;         // �Ƿ�ʹ��GPU�������Ⱦ
    std::atomic<bool>       m_dxvaCbEnabled = true;     // ���øñ�־λ����dxva2_release_buffer����
};

class DecodeThread : public QThread
{
    Q_OBJECT
public:
    explicit DecodeThread(IPlayerCore *iPlayerCore);
    virtual ~DecodeThread() Q_DECL_OVERRIDE;
    void stop();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    std::atomic<bool> m_isContinue = true;  // �Ƿ��������
    IPlayerCore*      m_playerCore   = nullptr;
};

class RenderThread : public QThread
{
    Q_OBJECT
public:
    explicit RenderThread(QWidget *wid, int duration = -1);
    virtual ~RenderThread() Q_DECL_OVERRIDE;
    void setFFmpeg(IPlayerCore *pFFmpeg);
    void stop();
    void pause(bool isPause);
	void setDstWinHandle(int handle);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void draw(QImage image, const QString &url);

private:
    std::atomic_bool m_isContinue = false;  // �Ƿ������Ⱦ
    IPlayerCore*     m_playerCore;          // ��������
    QWidget*         m_wid;                 // Ŀ�괰��
    int              m_duration;            // ��Ⱦ���
    std::atomic_bool m_isPause   = false;   // �Ƿ���ͣ��Ⱦ
    int              m_winHandle = 0;       // Ŀ�괰�ھ��
    QMutex           m_iPlayerCoreMtx;  // �������ͬʱ��д�����̺߳���Ⱦ�̣߳�
};

class Outputer : public QObject
{
    Q_OBJECT
public:

	explicit Outputer(IPlayerCore* iPlayerCore)
        : m_iPlayerCore(iPlayerCore)
        , QObject(Q_NULLPTR)
        , m_pFmtCtx(Q_NULLPTR)
        , m_vs(Q_NULLPTR)
        , m_vfc(0) {}
    ~Outputer(){close();}

    void open(AVFormatContext *iFmtCtx, const QString &url);
    void close();
    void write(const AVPacket *pkt);

private:
    QString          m_url;          // ���url
    AVFormatContext* m_pFmtCtx;      // �����װ��
    AVStream*        m_vs;           // ��Ƶ�����
    AVRational       m_inTimebase;   // ����ʱ��
    AVRational       m_outTimebase;  // ���ʱ��
    qint64           m_vfc;          // ��Ƶ֡����(Video Frame Count)
    bool m_headerWritten = false;  // avformat_write_header���óɹ����ܵ���av_write_trailer
    IPlayerCore* m_iPlayerCore = nullptr;
    QString      m_codecName;
};

