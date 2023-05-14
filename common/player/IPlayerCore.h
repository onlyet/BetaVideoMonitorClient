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

    // 设置播放器的渲染控件（QLabel的句柄）
    void setWindowHandle(HWND hwnd);

    // 使用DXVA硬解
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

    // 修改合并视频需要用到的txt
    void appendConcatFile(const QString &videoPath);
    void concatVideo(const QString& videoPath);

signals:
    void draw(QImage img, const QString &url);
    void begin(void *ptr);              // 通知窗口已开始

	// 已打开输入
	void inputOpened();

    void IpcParamsError(const QString &msg);
    void setNoError();

    void resolutionChanged();

    void concatStarted();
    void concatDone();

private:
    QString             m_path;                         // 输入URL
    DecodeThread       *m_decoder = nullptr;            // 读取解码线程
    RenderThread        *m_render = nullptr;            // 渲染线程
    AVFormatContext     *m_pAVFormatCtx = nullptr;      // 解封装器
    AVCodecContext      *m_vDecodeCtx = nullptr;        // 视频解码器上下文
    SwsContext          *m_swsCtx = nullptr;
    AVFrame             *m_pYuv = nullptr;              // 解码后的视频帧数据
    QMutex              m_mutex;                        // 互斥变量，多线程时避免同时间的读写AVFormatContext和AVCodecContext
    QMutex              m_mutex_output;                 // 输出列表数据锁
    QMap<QString, Outputer*> m_outputs;                 // 输出列表
    QString             m_err;                          // 错误信息
    bool                m_isEof;                        // 是否读完;
    double              m_fps;                          // 视频流帧率
    bool                m_isOpened;                     // 是否已打开媒体
    int                 m_videoStream;                  // 视频流类型
    QSize               m_size;                         // 当前视频尺寸
    AVPixelFormat       m_pixFmt;                       // 当前视频格式
    bool                m_enableDecode = true;          // 是否解码，主码流不解码，打开大屏则小屏不解码
	QString			    m_codecName;			        // 编码器名字

    int                 m_codecCtxWidth = 0;
    int                 m_codecCtxHeight = 0;
    bool                m_resolutionChanged = false;
    std::atomic_bool    m_destructed = false;

#ifdef Enable_D3dRender
    CD3DVidRender	    m_d3d;
    int                 m_yuvSize;
    BYTE                *m_yuvBuf = nullptr;
#endif

    // dxva渲染添加
public:
    HWND                    m_windowHandle;
    D3DPRESENT_PARAMETERS   m_d3dpp = { 0 };
    IDirect3DSurface9       *m_pBackBuffer = NULL;
    InputStream             *m_inputStream = nullptr;
    bool                    m_hwDecode = false;         // 是否使用GPU解码和渲染
    std::atomic<bool>       m_dxvaCbEnabled = true;     // 利用该标志位避免dxva2_release_buffer崩溃
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
    std::atomic<bool> m_isContinue = true;  // 是否继续解码
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
    std::atomic_bool m_isContinue = false;  // 是否继续渲染
    IPlayerCore*     m_playerCore;          // 播放引擎
    QWidget*         m_wid;                 // 目标窗口
    int              m_duration;            // 渲染间隔
    std::atomic_bool m_isPause   = false;   // 是否暂停渲染
    int              m_winHandle = 0;       // 目标窗口句柄
    QMutex           m_iPlayerCoreMtx;  // 避免多线同时读写（主线程和渲染线程）
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
    QString          m_url;          // 输出url
    AVFormatContext* m_pFmtCtx;      // 输出封装器
    AVStream*        m_vs;           // 视频输出流
    AVRational       m_inTimebase;   // 输入时基
    AVRational       m_outTimebase;  // 输出时基
    qint64           m_vfc;          // 视频帧数量(Video Frame Count)
    bool m_headerWritten = false;  // avformat_write_header调用成功才能调用av_write_trailer
    IPlayerCore* m_iPlayerCore = nullptr;
    QString      m_codecName;
};

