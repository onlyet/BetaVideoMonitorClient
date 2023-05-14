#include "GlobalConfig.h"
#include <util.h>
#include <CMessageBox.h>

#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QStandardpaths>
#include <QDateTime>
#include <QDir>
#include <QStorageInfo>

#define Screen24    qstr("24人画面")
#define Screen12    qstr("12人画面")
#define Screen8     qstr("8人画面")
#define Screen28    qstr("28人画面")

GlobalConfig::GlobalConfig(QObject *parent)
	: QObject(parent)
    , m_screenMap {
    {24, Screen24},
    {12, Screen12},
    {8, Screen8},
    {28, Screen28}
}
, m_screenList {
    Screen24,
    Screen12,
    Screen8,
    Screen28
}
{
}

GlobalConfig::~GlobalConfig()
{
}

bool GlobalConfig::isRecordDirExist(QSettings &setting)
{
    m_recordPath = setting.value(RecordPath).toString();
    QString oldRecordPath = m_recordPath;
    QDir dir(m_recordPath);
    if (m_recordPath.isEmpty())
    {
        m_recordPath = "D:/record/taguojiang";
        qInfo() << QStringLiteral("录制目录为空，创建默认目录: %1").arg(m_recordPath);
        if (!dir.mkpath(m_recordPath))
        {
            QString msg = QStringLiteral("创建默认录制目录%1失败").arg(m_recordPath);
            qInfo() << msg;
            CMessageBox::info(msg);
            return false;
        }
    }
    else if (!dir.exists())
    {
        if (m_recordPath.length() >= 3)
        {
            QString drive = m_recordPath.left(3);
            if (!util::isDriveExist(drive))
            {
                drive.replace(drive, "D:/", Qt::CaseInsensitive);
                if (!util::isDriveExist(drive))
                {
                    drive.replace(drive, "C:/", Qt::CaseInsensitive);
                }
                m_recordPath.replace(m_recordPath.left(3), drive, Qt::CaseInsensitive);
            }
        }

        qInfo() << QStringLiteral("创建目录：%1").arg(m_recordPath);
        if (!dir.mkpath(m_recordPath))
        {
            QString msg = QStringLiteral("创建录制目录%1失败").arg(m_recordPath);
            qInfo() << msg;
            CMessageBox::info(msg);
            return false;
        }
    }

    if (oldRecordPath != m_recordPath)
    {
        setting.setValue(RecordPath, m_recordPath);
    }
    return true;
}

GlobalConfig& GlobalConfig::instance()
{
	static GlobalConfig s_instance;
	return s_instance;
}

bool GlobalConfig::init()
{
	QString path = QString("%1/%2.ini").arg(QCoreApplication::applicationDirPath()).arg(util::appName());
	QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    QFile file(path);
	if (!file.exists())
	{
		if (!file.open(QIODevice::ReadWrite))
		{
			qWarning() << QString("Can not open file: %1").arg(path);
            QString msg = QStringLiteral("无法打开配置文件，程序退出");
            qWarning() << msg;
            CMessageBox::info(msg);
			return false;
		}
	}

    m_reduceAnalyzeTime = setting.value(ReduceAnalyzeTime, false).toBool();
    m_enableAudioStream = setting.value(EnableAudioStream, false).toBool();
    m_enableServer = setting.value(EnableServer, true).toBool();
    m_gpuSeatCnt = setting.value(GpuSeatCnt, 20).toInt();
    if (m_gpuSeatCnt < 0 || m_gpuSeatCnt > 28)
    {
        m_gpuSeatCnt = 20;
    }
    m_logoType = setting.value(LogoType, "logo1").toString();
    setLogo();
    m_rtspOverTcp = setting.value(RtspOverTcp, true).toBool();
    m_disableStartBtnTime = setting.value(DisableStartBtnTime).toInt();
    m_aPageSeatCnt = setting.value(APageSeatCnt, 24).toInt();

    if (1 && util::haveNvidiaGpu())
    {
        qInfo() << qstr("前%1个座位用GPU解码渲染").arg(m_gpuSeatCnt);
        qInfo() << qstr("大屏使用GPU解码渲染");
    }
    else
    {
        qInfo() << qstr("大屏使用CPU解码渲染");
    }

    if (!isRecordDirExist(setting) || !isDiskSpaceEnough())
    {
        return false;
    }

    setLogoPixmap(QPixmap(":/logo2.jpg"));

	return true;
}

bool GlobalConfig::load(const QString &path)
{
	return true;
}

bool GlobalConfig::addOutput()
{
	return true;
}

bool GlobalConfig::reduceAnalyzeTime()
{
    return m_reduceAnalyzeTime;
}

bool GlobalConfig::audioStreamEnabled()
{
    return m_enableAudioStream;
}

bool GlobalConfig::enableServer()
{
    return m_enableServer;
}

QString GlobalConfig::recordPath()
{
    return m_recordPath;
}

void GlobalConfig::setRecordPath(const QString &path)
{
    m_recordPath = path;
    util::setSetting(RecordPath, m_recordPath);
}

void GlobalConfig::enableRecordVideo(bool enabled)
{
    m_recordEnabled = enabled;
}

bool GlobalConfig::recordVideoEnabled()
{
    return m_recordEnabled;
}

bool GlobalConfig::isDiskSpaceEnough()
{
    QStorageInfo si(m_recordPath);
    float available = (float)si.bytesAvailable() / 1024 / 1024 / 1024;
    float total = (float)si.bytesTotal() / 1024 / 1024 / 1024;
    float percent = available / total;
    qInfo() << QStringLiteral("录制视频目录%1所在磁盘的可用空间为%2%").arg(m_recordPath).arg((int)(percent * 100));
#if 0
    if (percent <= 0.1)
    {
        m_recordEnabled = false;
        CMessageBox::info(qstr("%1盘空间不足!\n清理磁盘或修改录制视频保存路径").arg(m_recordPath.left(1)));
        return false;
    }
#endif
    return true;
}

void GlobalConfig::setLogo()
{
    if ("logo1" == m_logoType)
    {
        m_loginLogo = ":/logo1.png";
        m_seatLogo = ":/logo1.png";
    }
    else
    {
        m_loginLogo = "";
        m_seatLogo = "";
    }
}

int GlobalConfig::gpuSeatCnt() const
{
    return m_gpuSeatCnt;
}

QString GlobalConfig::logoType() const
{
    return m_logoType;
}

QString GlobalConfig::loginLogo() const
{
    return m_loginLogo;
}

QString GlobalConfig::seatLogo() const
{
    return m_seatLogo;
}

bool GlobalConfig::isRtspOverTcp() const
{
    return m_rtspOverTcp;
}

int GlobalConfig::startBtnDisabledTime() const
{
    return m_disableStartBtnTime;
}

QString GlobalConfig::defaultScreenNumText()
{
    if (!m_screenMap.contains(m_aPageSeatCnt))
    {
        m_aPageSeatCnt = 24;
    }
    return m_screenMap[m_aPageSeatCnt];
}

QStringList GlobalConfig::screenList() const
{
    return m_screenList;
}

void GlobalConfig::setThemeInfo(const QVariantList& themeList)
{
    if (themeList.isEmpty())
    {
        m_isDefaultTheme = true;
        return;
    }
    m_themeList = themeList;
}

QVariantList GlobalConfig::themeInfo()
{
    return m_themeList;
}

QString GlobalConfig::themeValue(const QString& key)
{
    for (const auto& var : m_themeList)
    {
        QVariantMap theme = var.toMap();
        if (theme["configKey"].toString() == key)
        {
            return theme["configValue"].toString();
        }
    }
    return "";
}

void GlobalConfig::setLogoPixmap(const QPixmap& pix)
{
    m_logoPixmap = pix;
}

QPixmap GlobalConfig::logoPixmap()
{
    return m_logoPixmap;
}

QString GlobalConfig::themeColorValue()
{
    QString color = themeValue("theme");
    if (color.isEmpty())
    {
        color = "#2B2B2B";
    }
    return color;
}

QString GlobalConfig::themeBgColor()
{
    QString color = themeValue("theme");
    if (color.isEmpty())
    {
        color = "#2B2B2B";
    }
    QString colorQss = QString("background-color:%1;").arg(color);
    return colorQss;
}

QString GlobalConfig::themeFontColor()
{
    QString color = themeValue("theme");
    if (color.isEmpty())
    {
        color = "#2B2B2B";
    }
    QString colorQss = QString("color:%1;").arg(color);
    return colorQss;
}

QString GlobalConfig::cancelBtnColor()
{
    QString color = themeValue("theme");
    if (color.isEmpty())
    {
        color = "#2B2B2B";
    }
    QString colorQss = QString("QPushButton{color:%1;} QPushButton:hover{background:%1;color:white;}").arg(color);
    return colorQss;
}
