#pragma once

#include <QObject>
#include <QVariant>
#include <QPixmap>

constexpr char KEY_LOGO[] = "logo";

class QSettings;

class GlobalConfig : public QObject
{
	Q_OBJECT

public:
	static GlobalConfig& instance();

	bool init();

	bool load(const QString &path = "");
	bool addOutput();

    bool reduceAnalyzeTime();
    bool audioStreamEnabled();
    bool enableServer();

    // 视频录制路径
    QString recordPath();
    void setRecordPath(const QString &path);
    void enableRecordVideo(bool enabled);
    bool recordVideoEnabled();

    bool gpuAccelerated() const { return m_gpuAccelerated; }
    int  gpuScreenCnt() const { return m_gpuScreenCnt; }

    QString logoType() const;
    QString loginLogo() const;
    QString seatLogo() const;

    bool isRtspOverTcp() const;

    int startBtnDisabledTime() const;

    QString defaultScreenNumText();
    QStringList screenList() const;

    void setThemeInfo(const QVariantList& themeList);
    QVariantList themeInfo();
    QString themeValue(const QString& key);

    void setLogoPixmap(const QPixmap& pix);
    QPixmap logoPixmap();

    // 背景颜色
    QString themeColorValue();
    QString themeBgColor();
    QString themeFontColor();

    QString cancelBtnColor();


private:
	GlobalConfig(QObject *parent = Q_NULLPTR);
	~GlobalConfig();

    bool isRecordDirExist(QSettings &setting);

    // 检查磁盘空间
    bool isDiskSpaceEnough();

    void setLogo();

private:
    bool                  m_reduceAnalyzeTime;
    bool                  m_enableAudioStream;
    bool                  m_enableServer;          // 是否有服务器
    QString               m_recordPath;            // 录制视频保存路径
    bool                  m_recordEnabled = true;  // 是否允许录制视频
    bool                  m_gpuAccelerated;
    int                   m_gpuScreenCnt;            // 前m_gpuSeatCnt个座位使用GPU解码渲染
    QString               m_logoType;
    QString               m_loginLogo;
    QString               m_seatLogo;
    bool                  m_rtspOverTcp;
    int                   m_disableStartBtnTime = 0;  // 按钮禁用时间（ms）
    int                   m_aPageSeatCnt;             // 默认一页显示座位数
    QMap<qint32, QString> m_screenMap;
    QStringList           m_screenList;
    bool                  m_isDefaultTheme = false;
    QVariantList          m_themeList;
    QPixmap               m_logoPixmap;
};

#define CONFIG GlobalConfig::instance()