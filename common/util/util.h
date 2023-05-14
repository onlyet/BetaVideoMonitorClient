#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QVariant>
#include <QSettings>
#include <QEventLoop>
#include <QJsonObject> 

#define APPNAME "BetaVideoMonitorClient"

#define qstr QStringLiteral
#define DATETIME_FORMAT_DEFAULT         "yyyy-MM-dd hh:mm:ss"

#define RecordPath                      "Volatile/RecordPath"            // ¼�񱣴�·��

#define EnableServer                    "Server/EnableServer"

#define ReduceAnalyzeTime               "Common/ReduceAnalyzeTime"
#define GpuSeatCnt                      "Common/GpuSeatCnt"
#define UseVectoredExceptionHandler     "Common/UseVectoredExceptionHandler"
#define LogoType                        "Common/LogoType"
#define RtspOverTcp                     "Common/RtspOverTcp"
#define DisableStartBtnTime             "Common/DisableStartBtnTime"
#define EnableAudioStream               "Common/EnableAudioStream"
#define APageSeatCnt                    "Common/APageSeatCnt"

const int LoopSuccess = 0;
const int LoopTimeout = -1;
const int LoopError = -2;

class QLabel;
class QWidget;

namespace util {
    /*! @brief �������ļ��ж�ȡָ������ */
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant(), const QString& filename = "");
    /*! @brief �޸����� */
    void setSetting(const QString& key, const QVariant& value, const QString& filename = "");

    /*! @brief ��ǰ�߳̿����¼�ѭ���ȴ�msecs����, ����QEventLoopִ�н�� */
    int esleep(QEventLoop* loop, int msecs);

    /*! @brief ʹ����Ψһ���ڣ�����Ѵ��ڷ���false */
    bool setProgramUnique(const QString& name);

    /*! @brief ��������ļ�·�������غ���������ļ�·�����������·���������򴴽���isCover��ʾ�Ƿ�ɾ������ */
    QString checkFile(const QString& filepath, bool isCover = false);

    /*! @brief ѭ��ɾ��Ŀ¼. �޷�ɾ������ʹ�õ��ļ���Ŀ¼ */
    bool rmDir(const QString& path);

    qint64 mSecsSinceEpoch();

    QString localIpv4();

    QMap<QString, QString> localIpv4Map();

    bool ensureDirExist(const QString& dirPath);

    // app�汾��
    QString appVersion();

    int screenWidth();
    int screenHeight();

    int scaleWidthByResolution(int width);
    QSize newSize(QSize size);

    void resetMaxWidth(QLabel* label, int maxWidth);

    // QLabel��ʾ�ı����������������ʾʡ�Ժ�
    void setTextWithEllipsis(QLabel* label, const QString& text);

    QString logDir();

    // �Ƿ���Ӣΰ���Կ�
    bool haveNvidiaGpu();

    QString formatTime(int secs, const QString& format);
    QStringList secToTime(int secs);
    QString timestamp2String(qint64 ms);

    QString QVariant2QString(const QVariant& map);
    QVariant QString2QVariant(const QString& s);

    QString logRtspUrl(const QString& url);
    QString logRtspError(const QString& url);

    bool isDriveExist(const QString& drive);

    bool isFileExist(const QString& path);

    QStringList filePathListInDir(const QString& dirPath, QStringList filter = QStringList());

    QString appName();

    void setRetainSizeWhenHidden(QWidget* w, bool retain = true);
};

#endif // UTIL_H
