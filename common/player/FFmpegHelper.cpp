#include "FFmpegHelper.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#if defined (M_OS_64) || defined (Q_OS_WIN64)
    typedef quint64 xhandle;
#else
    typedef quint32 xhandle;
#endif

// 错误登记设置成AV_LOG_ERROR。AV_LOG_WARNING的话可能会输出大量的deprecated日志（deprecated pixel format used, make sure you did set range correctly）
#define FFmpegLogLevel AV_LOG_ERROR

void FFmpegLogFunc(void *ptr, int level, const char *fmt, va_list vl) 
{
    if (level > FFmpegLogLevel)
    {
        return;
    }
    char buf[512] = { 0 };
    vsprintf_s(buf, 512, fmt, vl);
    qDebug() << "err:" << buf;
}

static bool isFFmpegInited = false;
void FFmpegHelper::init_ffmpeg()
{
    if(isFFmpegInited) return;
    av_register_all();
    avdevice_register_all();
    avformat_network_init();
    // 将FFmpeg日志写到文件
    av_log_set_callback(FFmpegLogFunc);
    av_log_set_level(FFmpegLogLevel);
    qInfo() << "FFmpeg" << av_version_info();
    isFFmpegInited = true;

}

QString FFmpegHelper::parseErr(int errNum)
{
    char err[1024] = {0};
    av_strerror(errNum, err, 1024);
    return err;
}

double FFmpegHelper::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)(r.num) / (double)(r.den);
}

QString FFmpegHelper::getPtrString(void *ptr)
{
    return QString("%1").arg((xhandle)ptr, sizeof(xhandle)*2, 16, QChar('0')).toUpper();
}

QString FFmpegHelper::checkFile(const QString &filepath, bool isCover)
{
    QFileInfo finfo(filepath);
    QString suffix = finfo.completeSuffix();
    QString name = finfo.baseName();
    QString path = finfo.absolutePath();
    QDir dir;
    if(!dir.mkpath(path))
    {
        qWarning() << QStringLiteral("创建路径失败：%1").arg(path);
    }

    QString url = filepath;
    QFile file(url);
    if (isCover)
    {
        if (file.exists())
        {
            file.remove();
        }
    }
    else
    {
        int i = 0;
        while(file.exists())
        {
            ++i;
            url = QString("%1/%2_%3.%4").arg(path).arg(name).arg(i).arg(suffix);
            file.setFileName(url);
        }
    }
    return url;
}
