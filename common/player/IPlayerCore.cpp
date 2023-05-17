#include "IPlayerCore.h"
#include "FFmpegHelper.h"
#include "ffmpeg_dxva2.h"

#include <GlobalConfig.h>
#include <GlobalSignalSlot.h>
#include <util.h>
#include <ErrorCode.h>

#include <QDebug>
#include <QDateTime>
#include <QAudioOutput>
#include <QThreadPool>
#include <QEventLoop>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QMap>
#include <QFuture>
#include <QtConcurrent>

#define IS_SET_FPS_WHEN_ERROR   1   ///< 当无法解析帧率时, 是否使用自定义的数值
#define FPS_CUSTOM              25  ///< 当无法解析帧率时使用的自定义数值

AVPixelFormat GetHwFormat(AVCodecContext *s, const AVPixelFormat *pix_fmts)
{
    //InputStream* ist = (InputStream*)s->opaque;
    IPlayerCore* iPlayerCore = (IPlayerCore*)s->opaque;
    if (nullptr == iPlayerCore)
    {
        return AV_PIX_FMT_NONE;
    }
    InputStream *ist = iPlayerCore->m_inputStream;
    if (nullptr == ist)
    {
        return AV_PIX_FMT_NONE;
    }
    ist->active_hwaccel_id = HWACCEL_DXVA2;
    ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
    return ist->hwaccel_pix_fmt;
}

IPlayerCore::IPlayerCore(QWidget *wid)
    : QThread(Q_NULLPTR)
    , m_mutex(QMutex::Recursive)
{
    FFmpegHelper::init_ffmpeg();

    init();

    m_decoder = new DecodeThread(this);
    if (wid)
    {
        m_render = new RenderThread(wid, 40);
        connect(m_render, &RenderThread::draw, this, &IPlayerCore::draw);
        m_render->setFFmpeg(this);
    }

    moveToThread(this);
    start();
}

IPlayerCore::~IPlayerCore()
{
    //qDebug() << "~IPlayerCore() 1";
    if(m_decoder)
    {
        delete m_decoder;
        m_decoder = Q_NULLPTR;
    }

    if(m_render)
    {
        delete m_render;
        m_render = Q_NULLPTR;
    }

    m_mutex_output.lock();
    qDeleteAll(m_outputs);
    m_outputs.clear();
    m_mutex_output.unlock();

    m_destructed = true;
    qDebug() << "~IPlayerCore() 2";
}

void IPlayerCore::init()
{
    if (m_vDecodeCtx)
    {
        m_vDecodeCtx->opaque = nullptr;
        avcodec_free_context(&m_vDecodeCtx);
        m_vDecodeCtx = nullptr;
    }
    if (m_pAVFormatCtx)
    {
        avformat_close_input(&m_pAVFormatCtx); // 关闭媒体
        m_pAVFormatCtx = Q_NULLPTR;
    }
    if (m_swsCtx)
    {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }

    {
        //QMutexLocker lock(&m_decodeFrameMtx);
        if (m_pYuv)
        {
            av_frame_free(&m_pYuv); // 关闭时释放解码后的视频帧空间
            m_pYuv = Q_NULLPTR;
        }
    }

#ifdef Enable_D3dRender
#ifdef Enable_Hardcode
	// NV12
#else
    if (m_frameBuf)
    {
        delete[] m_frameBuf;
        m_frameBuf = nullptr;
        m_frameSize = 0;
    }
#endif
#endif

    // 数据初始化
    m_fps = 0;
    m_isEof = false;

    m_videoStream = -1;
    m_size = QSize(0,0);

    m_isOpened = false;
}

void IPlayerCore::open(const QString &path)
{
    if (doOpen(path))
    {
        emit begin(Q_NULLPTR);
    }
}

void IPlayerCore::pause(bool isPause)
{
    if (m_render) m_render->pause(isPause);
}

bool IPlayerCore::doOpen(const QString &path)
{
    QTime t = QTime::currentTime();
	if (m_isOpened)
	{
		doClose(); // 打开前先关闭清理
	}
    if(path.isEmpty())
    {
        return false;
    }

    m_path = path;
    AVDictionary* options = Q_NULLPTR;
    if (m_path.startsWith("rtsp"))
    {
        av_dict_set(&options, "stimeout", "10000000", 0);   // 10秒，太小的话avformat_open_input会返回Operation not permitted
        av_dict_set(&options, "max_delay", "100000", 0);    // 100毫秒

        if (CONFIG.isRtspOverTcp())
        {
            av_dict_set(&options, "rtsp_transport", "tcp", 0);
        }
        else
        {
            av_dict_set(&options, "rtsp_transport", "udp", 0);
            // UDP协议需要增大buffer_size，不然播放会出现花屏,10M，TCP应该不需要设置
            av_dict_set(&options, "buffer_size", "10240000", 0);
        }
    }

    AVInputFormat *iformat = Q_NULLPTR;
    QByteArray burl = m_path.toUtf8();
    int re = 0;

	//QTime t2 = QTime::currentTime();
    QMutexLocker locker(&m_mutex);

    QString errMsg = util::logRtspError(m_path);
    re = avformat_open_input(&m_pAVFormatCtx, burl.constData(), iformat, &options); // 打开解封装器
    if (re != 0)
    {
        QString err = parseError(re);
        qWarning() << "Open" << util::logRtspUrl(path) << "Fail:" << err;
        //if ("Error number -138 occurred" == err)
        //{
        errMsg.append(qstr("\n网络问题：连接摄像头超时"));
        qInfo() << errMsg;
        emit IpcParamsError(errMsg);
        //}
        doClose();
        return false;
    }
	//qDebug() << "avformat_open_input time: " << t2.elapsed();

    if (m_pAVFormatCtx->duration <= 0)    // 如果读取到的媒体信息有误
    {
        // 读取文件信息（有些媒体文件在打开时读不到，需要使用此函数）
        //QTime t3 = QTime::currentTime();
        if (1 && CONFIG.reduceAnalyzeTime())
        {
            // 启用这两个选项会获取不到fps
            m_pAVFormatCtx->probesize = 1024;
            m_pAVFormatCtx->max_analyze_duration = 1000;
        }
		re = avformat_find_stream_info(m_pAVFormatCtx, Q_NULLPTR);
        if (re < 0)
        {
            qWarning() << "Get Media Info Error:" << parseError(re);
            doClose();
            return false;
        }
		//qDebug() << "avformat_find_stream_info time: " << t3.elapsed();
	}

	// 打开解码器
    int ret = 0;
    QString vcodecName, acodecName;
    int64_t videoBitrate;
    for (int i = 0 ; i < m_pAVFormatCtx->nb_streams; ++i)
    {
        AVStream* stream = m_pAVFormatCtx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            AVCodecID codecId = codecpar->codec_id;
            AVCodec* decoder = avcodec_find_decoder(codecId);
            if (!decoder)
            {
                qWarning("Video Codec Not Found![%d]", codecId);
                doClose();
                return false;
            }

            //从视频流中拷贝参数到codecCtx
            m_vDecodeCtx = avcodec_alloc_context3(decoder);
            if ((ret = avcodec_parameters_to_context(m_vDecodeCtx, codecpar)) < 0)
            {
                qDebug() << "Video avcodec_parameters_to_context failed,error code: " << ret;
                doClose();
                return false;
            }
            m_vDecodeCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
            // 尝试显示缩略图
            m_vDecodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            if (m_hwDecode)
            {
                if (NULL == m_windowHandle)
                {
                    qDebug() << QStringLiteral("Window handle NUll, check code");
                    doClose();
                    return false;
                }

                // dxva初始化需要，demo是（1920x1088）
                m_vDecodeCtx->coded_width = m_vDecodeCtx->width;
                m_vDecodeCtx->coded_height = m_vDecodeCtx->height;

                m_vDecodeCtx->thread_count = 1;  // Multithreading is apparently not compatible with hardware decoding
                m_inputStream = new InputStream();
                m_inputStream->hwaccel_id = HWACCEL_AUTO;
                m_inputStream->active_hwaccel_id = HWACCEL_AUTO;
                m_inputStream->hwaccel_device = const_cast<char*>("dxva2");
                m_inputStream->dec = decoder;
                m_inputStream->dec_ctx = m_vDecodeCtx;

                m_vDecodeCtx->opaque = this;
                if (dxva2_init(m_vDecodeCtx, m_windowHandle) != 0)
                {
                    qDebug() << "dxva2_init failed";
                    doClose();
                    return false;
                }
                m_vDecodeCtx->get_buffer2 = m_inputStream->hwaccel_get_buffer;
                m_vDecodeCtx->get_format = GetHwFormat;
                m_vDecodeCtx->thread_safe_callbacks = 1;
            }
            else
            {
                m_vDecodeCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
            }

            m_videoStream = i;
            stream->r_frame_rate;
            m_fps = FFmpegHelper::r2d(stream->avg_frame_rate);
            m_size = QSize(m_vDecodeCtx->width, m_vDecodeCtx->height);
            m_codecCtxWidth = m_vDecodeCtx->width;
            m_codecCtxHeight = m_vDecodeCtx->height;
            m_pixFmt = m_vDecodeCtx->pix_fmt;

			int err = avcodec_open2(m_vDecodeCtx, decoder, Q_NULLPTR);
            if (0 != err)
            {
                qWarning() << "Open video decoder Error:" << parseError(err);
                doClose();
                return false;
            }
            vcodecName = decoder->name;
			m_codecName = vcodecName;
            videoBitrate = codecpar->bit_rate / 1000;
        }
    }

    locker.unlock();

    int w = m_size.width();
    int h = m_size.height();
    qDebug().nospace() << QString("Video size: %1x%2, FPS: %3, type: %4, pixFmt: %5")
        .arg(w).arg(h).arg(m_fps)
        .arg(vcodecName).arg(m_pixFmt)/*.arg(videoBitrate)*/;

    int errCode = ErrorCode_IPCOk;
    if (w > 1920 || h > 1080) {
        errMsg.append(qstr("\n分辨率太高:%1x%2").arg(w).arg(h));
        errCode |= ErrorCode_IPCResolutionError;
    }
    if ("h264" != vcodecName) {
        errMsg.append(qstr("\n视频编码不是H.264"));
        errCode |= ErrorCode_IPCCodecTypeError;
    }

    if (ErrorCode_IPCOk != errCode)
    {
        qInfo() << errMsg;
        emit IpcParamsError(errMsg);
        doClose();
        return false;
    }
    else
    {
        emit IpcParamsError("");
    }

	if (m_fps == 0)
	{
		qWarning() << "Can not get fps!!";
	}

#ifdef Enable_D3dRender
#ifdef Enable_Hardcode
	// NV12
#else
    m_frameSize = m_size.width() * m_size.height();
    m_frameBuf = new BYTE[m_frameSize * 3 / 2];
    memset(m_frameBuf, 0, m_frameSize * 3 / 2);
#endif
#endif

    if(m_decoder)
    {
        m_decoder->start();
    }
    if(m_render)
    {
        m_render->start();
    }

    qDebug() << QString("open %1, time: %2ms").arg(util::logRtspUrl(m_path)).arg(t.elapsed());
	m_isOpened = true;
	emit inputOpened();
    m_dxvaCbEnabled = true;
    m_resolutionChanged = false;
    return true;
}

void IPlayerCore::close()
{
    doClose();
}

void IPlayerCore::doClose()
{
    if (!m_path.isEmpty())
    {
        qDebug() << "Close" << util::logRtspUrl(m_path);
    }
    if (m_decoder)
    {
        m_decoder->stop();
    }
    if (m_render)
    {
        m_render->stop();
    }
    m_mutex_output.lock();
    qDeleteAll(m_outputs);
    m_outputs.clear();
    m_mutex_output.unlock();

    QMutexLocker locker(&m_mutex);

    if (m_hwDecode)
    {
        m_dxvaCbEnabled = false;

        if (m_vDecodeCtx)
        {
            dxva2_uninit(m_vDecodeCtx);
        }
        if (m_inputStream)
        {
            delete m_inputStream;
            m_inputStream = nullptr;
        }
    }

    init();
}

AVPacket IPlayerCore::read()
{
    AVPacket pkt;
    memset(&pkt,0,sizeof(AVPacket));

    if (!m_pAVFormatCtx)
    {
        return pkt;
    }
    int re = av_read_frame(m_pAVFormatCtx, &pkt); // 读取视频帧
    if (re != 0) // 读取失败
    {
        if (re == AVERROR_EOF)
        {
            m_isEof = true;// 文件读取结束
            // 避免一直EOF造成高CPU占用
            msleep(25);
        }
        qDebug() << QString("av_read_frame failed: %1, %2").arg(parseError(re)).arg(util::logRtspUrl(m_path));
    }

    return pkt;
}

int IPlayerCore::decode(const AVPacket *pkt)
{
    if (!m_pAVFormatCtx)
    {
        return 0;
    }

    // 任务管理器看到这里会增加内存
    int ret = avcodec_send_packet(m_vDecodeCtx, pkt);
    if (ret < 0)
    {
        qDebug() << "avcodec_send_packet failed, " << parseError(ret);
        return 0;
    }
    // web端修改IPC分辨率后，avcodec_send_packet后最先获取到分辨率的改变
    if (m_vDecodeCtx->width != m_codecCtxWidth || m_vDecodeCtx->height != m_codecCtxHeight)
    {
        // 避免分辨率改变导致的崩溃
        m_resolutionChanged = true;
        emit resolutionChanged();
        qWarning() << qstr("%1分辨率由%2x%3变成%4x%5").arg(util::logRtspUrl(m_path)).arg(m_codecCtxWidth).arg(m_codecCtxHeight)
            .arg(m_vDecodeCtx->width).arg(m_vDecodeCtx->height);
        return 0;
    }
       
    //QMutexLocker lock(&m_decodeFrameMtx);

    if (!m_pYuv)
    {
        m_pYuv = av_frame_alloc();
    }
    if (m_hwDecode)
    {
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(m_vDecodeCtx, m_pYuv);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                //qDebug() << "avcodec_receive_frame failed, " << parseError(ret);
                return 0;

            }
            else if (ret < 0)
            {
                qDebug() << "avcodec_receive_frame failed, " << parseError(ret);
                return ret;
            }

            //获取数据同时渲染
            dxva2_retrieve_data_call(m_vDecodeCtx, m_pYuv);
        }
    }
    else
    {
        ret = avcodec_receive_frame(m_vDecodeCtx, m_pYuv);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            qDebug() << "avcodec_receive_frame failed, " << parseError(ret);
            return 0;

        }
        else if (ret < 0)
        {
            qDebug() << "avcodec_receive_frame failed, " << parseError(ret);
            return ret;
        }

#ifdef Enable_D3dRender
#ifdef Enable_Hardcode
#else
        memcpy_s(m_frameBuf, m_frameSize, m_pYuv->data[0], m_frameSize);
        memcpy_s(m_frameBuf + m_frameSize, m_frameSize / 4, m_pYuv->data[1], m_frameSize / 4);
        memcpy_s(m_frameBuf + m_frameSize * 5 / 4, m_frameSize / 4, m_pYuv->data[2], m_frameSize / 4);
#endif // Enable_Hardcode
#endif
    }
  
    return 0;
}

bool IPlayerCore::toRgb(uchar* out, int outWidth, int outHeight) {
    if (!out || outWidth <= 0 || outHeight <= 0 || m_videoStream < 0) {
        return false;
    }

    try {
#ifdef Enable_D3dRender
        if (!m_pYuv || m_pYuv->linesize[0] == 0) {
            return false;
        }

        int intputWidth  = m_size.width();
        int intputHeight = m_size.height();
#ifdef Enable_Hardcode
        // m_d3d.Render_NV12(m_nv12Buf, intputWidth, intputHeight);
        return true;
#else
        m_d3d.Render_YUV(m_frameBuf, intputWidth, intputHeight);
        return true;
#endif  // !Enable_Hardcode
#endif  // !Enable_D3dRender

        QMutexLocker locker(&m_mutex);
        // 避免分辨率改变导致sws_scale奔溃
        if (m_resolutionChanged) {
            return false;
        }

        if (!m_pYuv || 0 == m_pYuv->linesize[0] || m_pYuv->width <= 0 || m_pYuv->height <= 0 ||
            AV_PIX_FMT_NONE == m_pYuv->format) {
            return false;
        }

        /*
         * pix_fmt为AV_PIX_FMT_NONE（-1）的时候会崩溃，并且没有dmp文件，调试模式下直接退出了
         * try-catch也捕获不到
         * 日志输出：QObject::~QObject: Timers cannot be stopped from another thread
         */
        m_swsCtx = sws_getCachedContext(m_swsCtx, m_pYuv->width, m_pYuv->height,
                                        (AVPixelFormat)m_pYuv->format, outWidth, outHeight,
                                        AV_PIX_FMT_BGRA, SWS_BICUBIC, nullptr, nullptr, nullptr);
        if (!m_swsCtx) {
            qDebug() << "sws_getCachedContext failed";
            return false;
        }

        uint8_t* data[AV_NUM_DATA_POINTERS] = {nullptr};
        data[0]                             = out;  // 第一位输出RGB
        int lineSize[AV_NUM_DATA_POINTERS]  = {0};
        lineSize[0] = outWidth * 4;  // 一行的宽度为4个字节（32位）

        int h = sws_scale(m_swsCtx,
                          m_pYuv->data,      // 当前处理区域的每个通道的数据指针
                          m_pYuv->linesize,  // 每个通道行字节数
                          0,
                          m_pYuv->height,  // 原视频帧的高度
                          data,            // 输出的每个通道数据指针
                          lineSize);       // 每个通道行字节数;

        return h > 0;
    } catch (std::exception& e) {
        qCritical() << "std::exception: " << e.what();
        return false;
    }
}

int IPlayerCore::stepVideoOnly()
{
    if (m_resolutionChanged)
    {
        msleep(40);
        return -1;
    }

    if (isEof())
    {
        msleep(40);
        return -1;
    }

    QMutexLocker locker(&m_mutex);

    AVPacket pkt = read();
    if (pkt.size <= 0)
    {
        av_packet_unref(&pkt);
        msleep(5);
        return -1;
    }

    int type = -1;
    if (pkt.stream_index == m_videoStream)
    {
        type = 0;
    }
    else
    {
        av_packet_unref(&pkt);
        return -1;
    }

    {
        QMutexLocker locker(&m_mutex_output);
        for (const auto &writer : m_outputs)
        {
            writer->write(&pkt);
        }
    }

    // 主码流不解码，大屏播放的时候小屏不解码
    if (!m_enableDecode)
    {
        av_packet_unref(&pkt);
        return 0;
    }

    decode(&pkt);
    av_packet_unref(&pkt);

    return type;
}

QString IPlayerCore::path() const
{
    return m_path;
}

bool IPlayerCore::isOpened() const
{
    return m_isOpened;
}

double IPlayerCore::getFps() const
{
    return m_fps ? m_fps : 25;
}

bool IPlayerCore::isEof() const
{
    return m_isEof;
}

QSize IPlayerCore::getSize() const
{
    return m_size;
}

void IPlayerCore::addOutput(const QString &url)
{
    if (url.isEmpty()) return;

    if (!isOpened())
    {
        qDebug() << "!isOpened()";
        return;
    }
    {
        QMutexLocker locker(&m_mutex_output);
        if (m_outputs.contains(url))
        {
            qDebug() << QString("m_outputs.contains: %1").arg(url);
            return;
        }
    }
    Outputer *pWriter = new Outputer(this);
    pWriter->open(m_pAVFormatCtx, url);
    {
        QMutexLocker locker(&m_mutex_output);
        m_outputs.insert(url, pWriter);
    }

    appendConcatFile(url);
}

void IPlayerCore::rmOutput(const QString &url)
{
    if (url.isEmpty()) return;

    if (m_destructed)
    {
        qDebug() << "IPlayerCore destructed";
        return;
    }

    Outputer *pWriter;
    {
        QMutexLocker locker(&m_mutex_output);
        if (!m_outputs.contains(url))
        {
            return;
        }
        pWriter = m_outputs.value(url);
        m_outputs.remove(url);
    }
    delete pWriter;
    // 正常结束录制才能合并视频
    if (!GSignalSlot.isCrashed()) {
        concatVideo(url);
    }
}

#ifdef Enable_D3dRender
bool IPlayerCore::initD3D_NV12(HWND hwnd, int img_width, int img_height) {
    m_d3d.InitD3D_NV12(hwnd, img_width, img_height);
    return true;
}

bool IPlayerCore::initD3D_YUVJ420P(HWND hwnd, int img_width, int img_height) {
    bool b = m_d3d.InitD3D_YUV(hwnd, img_width, img_height);
	if (!b)
	{
		qWarning() << "m_d3d.InitD3D_YUV failed";
	}
    return b;
}
#endif

void IPlayerCore::enableDecode(bool enabled)
{
    m_enableDecode = enabled;
}

QString IPlayerCore::codecName() const
{
	return m_codecName;
}

void IPlayerCore::setWindowHandle(HWND hwnd)
{
    m_windowHandle = hwnd;
}

void IPlayerCore::enableHwDecode()
{
    m_hwDecode = true;
}

void IPlayerCore::appendConcatFile(const QString& videoPath) {
    QTime t = QTime::currentTime();
    if (videoPath.isEmpty() || !videoPath.endsWith(".ts")) {
        return;
    }
    QFileInfo fi(videoPath);
    QString   baseName = fi.baseName();
    QString   txt  = QString("%1/%2.txt").arg(fi.absolutePath(), baseName);
    QFile file(txt);
    if (!file.open(QIODevice::ReadWrite /*| QIODevice::Append | QIODevice::Text*/)) {
        qDebug() << "concat file open failed";
    }
    int     suffix  = 0;
    QString suffixStr;
    QString content = file.readAll();
    QString name    = QString("%1.ts").arg(baseName);
    QString filePath = QString("%1/%2").arg(fi.absolutePath(), name);
    while (QFile::exists(filePath)) {
        if (!content.contains(name)) {
            QString line = QString("file \'%1%2.ts\'\n").arg(baseName, suffixStr);
            file.write(line.toUtf8());
        }
        suffixStr = QString("_%1").arg(++suffix);
        name = QString("%1%2.ts").arg(baseName, suffixStr);
        filePath = QString("%1/%2").arg(fi.absolutePath(), name);
    }
    qDebug() << "appendConcatFile duration:" << t.elapsed();
}

void IPlayerCore::concatVideo(const QString& videoPath) {
    if (videoPath.isEmpty() || !videoPath.endsWith(".ts")) {
        return;
    }

    emit concatStarted();
    //QtConcurrent::run([this, videoPath]() {
    QTime     t = QTime::currentTime();
    QFileInfo fi(videoPath);
    QString   absolutePath = fi.absolutePath();
    QString   baseName     = fi.baseName();
    QString   txt          = QString("%1/%2.txt").arg(absolutePath, baseName);
    if (QFile::exists(txt)) {
        // ffmpeg - f concat - safe 0 - i filelist.txt - c copy - y concat3.mp4
        QString   mp4     = QString("%1/%2.mp4").arg(absolutePath, baseName);
        QString   cmd     = QString("ffmpeg -f concat -safe 0 -i %1 -c copy -y %2").arg(txt, mp4);
        QProcess* process = new QProcess;
        process->setWorkingDirectory(QCoreApplication::applicationDirPath());
        process->start(cmd);
        // -1不会超时
        if (!process->waitForFinished(-1)) {
            qDebug() << QString("ffmpeg concat failed: %1").arg(process->error());
            delete process;
            emit concatDone();
            return;
        }
#if 0
            // ffmpeg只有标准错误没有标准输出
            QString err = QString::fromLocal8Bit(process->readAllStandardError());
            qDebug() << "err:" << err;
#endif
        delete process;

        // 删除txt和ts
        QDir dir(absolutePath);
        dir.remove(QString("%1.txt").arg(baseName));
        for (const QString& ts : dir.entryList(QStringList() << QString("%1*.ts").arg(baseName))) {
            dir.remove(ts);
        }
    }
    emit concatDone();
    qDebug() << "concatVideo duration:" << t.elapsed();
    //});
}

QString IPlayerCore::parseError(int errNum)
{
    m_err = FFmpegHelper::parseErr(errNum);
    return m_err;
}


DecodeThread::DecodeThread(IPlayerCore *iPlayerCore)
    : QThread(Q_NULLPTR)
    , m_playerCore(iPlayerCore) {
    moveToThread(this);
}

DecodeThread::~DecodeThread()
{
    stop();
    qDebug() << "~DecodeThread()";
}

void DecodeThread::stop()
{
    if(isRunning())
    {
        m_isContinue = false;
        quit();
        wait();
    }
}

void DecodeThread::run()
{
    if (!m_playerCore)return;
    m_isContinue = true;

    //QTime t;
    while (m_isContinue) {
        // 主码流不解码，子码流暂时也不解码音频
        //t.restart();
        m_playerCore->stepVideoOnly();
        int remainingTime = 1;
        if (m_isContinue && remainingTime > 0) {
            msleep(remainingTime);
        }
    }
    exec();
}

RenderThread::RenderThread(QWidget *wid, int duration)
    : QThread(Q_NULLPTR)
    , m_playerCore(Q_NULLPTR)
    , m_wid(wid)
    , m_duration(duration)
{
    m_winHandle = (int)wid->winId();
    moveToThread(this);
}

RenderThread::~RenderThread()
{
    stop();
    qDebug().noquote() << QString("Recycle Scaler(0x%1) for QWidget(0x%2)")
                          .arg(FFmpegHelper::getPtrString(this)).arg(FFmpegHelper::getPtrString(m_wid));
}
// 耗时
void RenderThread::setFFmpeg(IPlayerCore *pFFmpeg)
{
    QMutexLocker lock(&m_iPlayerCoreMtx);
    m_playerCore = pFFmpeg;
    m_duration = 0;
}

void RenderThread::stop()
{
    if(!isRunning()) return;

    m_isContinue = false;
    quit();
    wait();
}

void RenderThread::pause(bool isPause)
{
    m_isPause = isPause;
}

void RenderThread::setDstWinHandle(int handle)
{
	m_winHandle = handle; }

#ifdef Enable_D3dRender
bool RenderThread::initD3D_NV12(HWND hwnd, int img_width, int img_height) {
    m_d3d.InitD3D_NV12(hwnd, img_width, img_height);
    return true;
}

bool RenderThread::initD3D_YUVJ420P(HWND hwnd, int img_width, int img_height) {
    bool b = m_d3d.InitD3D_YUV(hwnd, img_width, img_height);
    if (!b) {
        qWarning() << "m_d3d.InitD3D_YUV failed";
    }
    return b;
}

void RenderThread::onD3d9Render() {

    if (!m_playerCore) return;
    int intputWidth  = m_playerCore->getSize().width();
    int intputHeight = m_playerCore->getSize().height();
#ifdef Enable_Hardcode
    // m_d3d.Render_NV12(m_nv12Buf, intputWidth, intputHeight);
#else
    m_d3d.Render_YUV(m_playerCore->frameBuf(), intputWidth, intputHeight);
#endif  // !Enable_Hardcode
}

#endif

void RenderThread::run() {
    qDebug() << "RenderThread started";
    if (!m_wid) {
        return;
    }
    m_isContinue = true;
    int w        = 0;
    int h        = 0;

#ifdef Enable_D3dRender
    while (m_isContinue) {
        w = m_playerCore->getSize().width();
        h = m_playerCore->getSize().height();
        if (!m_playerCore || 0 == w || 0 == h) {
            msleep(40);
        } else {
            break;
        }
    }

    if (m_playerCore) {
#ifdef Enable_Hardcode
        m_playerCore->initD3D_NV12((HWND)m_winHandle, w, h);
#else
        //if (!m_playerCore->initD3D_YUVJ420P((HWND)m_winHandle, w, h)) {
        if (!initD3D_YUVJ420P((HWND)m_winHandle, w, h)) {
            qDebug() << "RenderThread exited unexpectedly";
            return;
        }
#endif  // !Enable_Hardcode
    }

#endif  // !Enable_D3dRender

    QTime tt;
    while (m_isContinue) {
        QMutexLocker lock(&m_iPlayerCoreMtx);

        if (!m_playerCore) {
            msleep(40);
            continue;
        }
        if (m_duration <= 0) {
            m_duration = 1000 / m_playerCore->getFps();
        }

        tt.restart();

#ifdef Enable_D3dRender
        if (!m_isPause && m_wid->isVisible()) {
            onD3d9Render();
        }
#else
        if (!m_isPause && m_wid->isVisible()) {
            w = m_wid->width() / 2 * 2;
            h = m_wid->height() / 2 * 2;
            if (w > 0 && h > 0) {
                QImage img(w, h, QImage::Format_ARGB32);
                if (m_playerCore->toRgb(img.bits(), w, h))  // 很耗时，需要优化
                {
                    emit draw(img, m_playerCore->path());
                }
            }
        }
#endif  // !Enable_D3dRender

        lock.unlock();

        int remainingTime = m_duration - tt.elapsed();
        if (m_isContinue && remainingTime > 0) {
            msleep(remainingTime);
        }
    }

    qDebug() << "RenderThread exit";
}

void Outputer::open(AVFormatContext *iFmtCtx, const QString &url)
{
    close();
    bool flag = false;
    do {
        if(!iFmtCtx)
        {
			qDebug() << "Outputer::open AVFormatContext is null";
			QThread::msleep(100);
            break;
        }

		m_codecName = m_iPlayerCore->codecName();

        m_url = url;
        const char *ofmt = Q_NULLPTR;
        if (url.startsWith("rtsp://"))
        {
            ofmt = "rtsp";
        }
        else if (url.startsWith("udp://"))
        {
            ofmt = "h264";
        }
        else if (!url.contains("://"))
        {
            // 同名文件增加序号
            m_url = FFmpegHelper::checkFile(m_url, false); // 本地路径
        }

        QByteArray burl = m_url.toUtf8();
        const char *curl = burl.constData();

        int err = avformat_alloc_output_context2(&m_pFmtCtx, Q_NULLPTR, ofmt, curl);
        if (!m_pFmtCtx)
        {
            qWarning() << "Open Output Error:" << FFmpegHelper::parseErr(err);
            break;
        }

        for (int i = 0 ; i < iFmtCtx->nb_streams; i++)
        {
            AVStream* stream = iFmtCtx->streams[i];
            AVCodecParameters* srcCodecpar = stream->codecpar;

            if (srcCodecpar->codec_type == AVMEDIA_TYPE_VIDEO) // 如果是视频类型
            {
                m_inTimebase = iFmtCtx->streams[i]->time_base;
                m_vs = avformat_new_stream(m_pFmtCtx, nullptr);
                if (!m_vs)
                {
                    qWarning() << "Create Video Output Stream Failed";
                    break;
                }

                err = avcodec_parameters_copy(m_vs->codecpar, srcCodecpar);
                if (err < 0)
                {
                    qWarning() << "Copy Video Codec Parameter Error:" << FFmpegHelper::parseErr(err);
                    break;
                }
                m_vs->codec->codec_tag = 0;

                //if (m_pFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
                //{
                // 显示缩略图
                //m_vs->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                //}
            }
        }

        if (!(m_pFmtCtx->oformat->flags & AVFMT_NOFILE)) 
		{
            // 新建了视频文件
            err = avio_open(&m_pFmtCtx->pb, curl, AVIO_FLAG_WRITE);
            if (err < 0)
            {
                qWarning() << "avio_open" << curl << " Error:" << FFmpegHelper::parseErr(err);
                break;
            }
            qDebug() << "avio_open: " << url;
        }

        // FIXME: 某的摄像头会出现错误：Invalid argument
        err = avformat_write_header(m_pFmtCtx, Q_NULLPTR);
        if (err < 0)
        {
            qWarning() << "Write Head Error:" << FFmpegHelper::parseErr(err);
            break;
        }
        m_headerWritten = true;
        flag = true;
    }while(0);

    if(!flag) close();
}

void Outputer::close()
{
    if(m_pFmtCtx)
    {
        qDebug() << "close output" << m_url;
        if(m_headerWritten)
        {
            av_write_trailer(m_pFmtCtx);
        }
        if(m_pFmtCtx->pb)
        {
            avio_close(m_pFmtCtx->pb);
        }
        avformat_free_context(m_pFmtCtx);
        m_pFmtCtx = Q_NULLPTR;
    }
    m_vs = Q_NULLPTR;
    m_inTimebase = { 1, AV_TIME_BASE };
    m_vfc = 0;
}


/*
* 遇到问题：录制的视频没有进度条并快速播放，原因：pts为0
* 所以pts为0时则根据FPS_CUSTOM（25）计算pts，这时候IPC网页设置的帧率要和FPS_CUSTOM相同
*/
void Outputer::write(const AVPacket *pkt)
{
    if (!m_pFmtCtx)
    {
        return;
    }

    if (!m_vs)
    {
        return;
    }

    AVPacket *npkt = av_packet_clone(pkt);
    //qDebug() << QString("pts:%1,dts:%2,duration:%3").arg(npkt->pts).arg(npkt->dts).arg(npkt->duration);

    if ("hevc" == m_codecName)
    {
        if (0 == npkt->pts)
        {
            int duration = pkt->duration;
#if IS_SET_FPS_WHEN_ERROR
            if (duration == 0)
            {
                duration = m_inTimebase.den / FPS_CUSTOM;
            }
#endif // IS_SET_FPS_WHEN_ERROR
            npkt->pts = m_vfc * duration;
            npkt->dts = npkt->pts;
            npkt->duration = duration;
        }
    }
    else
    {
        av_packet_rescale_ts(npkt, m_inTimebase, m_vs->time_base);
    }

    int ret = av_interleaved_write_frame(m_pFmtCtx, npkt);
    if (0 > ret)
    {
        qWarning() << "write error:" << FFmpegHelper::parseErr(ret);
    }
    av_packet_unref(npkt);
    av_packet_free(&npkt);
    npkt = Q_NULLPTR;

    m_vfc++;
}

