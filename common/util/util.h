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

#define RecordPath                      "Volatile/RecordPath"            // 录像保存路径

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
    /*! @brief 从配置文件中读取指定配置 */
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant(), const QString& filename = "");
    /*! @brief 修改配置 */
    void setSetting(const QString& key, const QVariant& value, const QString& filename = "");

    /*! @brief 当前线程开启事件循环等待msecs毫秒, 返回QEventLoop执行结果 */
    int esleep(QEventLoop* loop, int msecs);

    /*! @brief 使程序唯一存在，如果已存在返回false */
    bool setProgramUnique(const QString& name);

    /*! @brief 检查完整文件路径，返回合理的完整文件路径。如果所属路径不存在则创建。isCover表示是否删除已有 */
    QString checkFile(const QString& filepath, bool isCover = false);

    /*! @brief 循环删除目录. 无法删除正在使用的文件和目录 */
    bool rmDir(const QString& path);

    qint64 mSecsSinceEpoch();

    QString localIpv4();

    QMap<QString, QString> localIpv4Map();

    bool ensureDirExist(const QString& dirPath);

    // app版本号
    QString appVersion();

    int screenWidth();
    int screenHeight();

    int scaleWidthByResolution(int width);
    QSize newSize(QSize size);

    void resetMaxWidth(QLabel* label, int maxWidth);

    // QLabel显示文本，超过最大宽度则显示省略号
    void setTextWithEllipsis(QLabel* label, const QString& text);

    QString logDir();

    // 是否有英伟达显卡
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
