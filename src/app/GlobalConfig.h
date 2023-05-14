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

    // ��Ƶ¼��·��
    QString recordPath();
    void setRecordPath(const QString &path);
    void enableRecordVideo(bool enabled);
    bool recordVideoEnabled();

    int gpuSeatCnt() const;

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

    // ������ɫ
    QString themeColorValue();
    QString themeBgColor();
    QString themeFontColor();

    QString cancelBtnColor();


private:
	GlobalConfig(QObject *parent = Q_NULLPTR);
	~GlobalConfig();

    bool isRecordDirExist(QSettings &setting);

    // �����̿ռ�
    bool isDiskSpaceEnough();

    void setLogo();

private:
    bool                  m_reduceAnalyzeTime;
    bool                  m_enableAudioStream;
    bool                  m_enableServer;          // �Ƿ��з�����
    QString               m_recordPath;            // ¼����Ƶ����·��
    bool                  m_recordEnabled = true;  // �Ƿ�����¼����Ƶ
    int                   m_gpuSeatCnt;            // ǰm_gpuSeatCnt����λʹ��GPU������Ⱦ
    QString               m_logoType;
    QString               m_loginLogo;
    QString               m_seatLogo;
    bool                  m_rtspOverTcp;
    int                   m_disableStartBtnTime = 0;  // ��ť����ʱ�䣨ms��
    int                   m_aPageSeatCnt;             // Ĭ��һҳ��ʾ��λ��
    QMap<qint32, QString> m_screenMap;
    QStringList           m_screenList;
    bool                  m_isDefaultTheme = false;
    QVariantList          m_themeList;
    QPixmap               m_logoPixmap;
};

#define CONFIG GlobalConfig::instance()