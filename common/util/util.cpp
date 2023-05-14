#include "util.h"

#include <winsock2.h>
#include <iphlpapi.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QSettings>
#include <QCryptographicHash>
#include <math.h>
#include <QTimer>
#include <QLocalSocket>
#include <QLocalServer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QApplication>
#include  <QNetworkInterface>
#include <QProcess>
#include <QDesktopWidget>
#include <QLabel>
#include <QStringList>

#include <DXGI.h>  

#pragma comment(lib, "IPHLPAPI.lib")

QVariant util::getSetting(const QString& key, const QVariant& defaultValue, const QString& filename)
{
    QString path;
    QString appName = util::appName();
    QString exeDirPath = QApplication::applicationDirPath();
    if (filename.isEmpty())
    {
        path = QString("%1/%2.ini").arg(exeDirPath).arg(appName);
    }
    else
    {
        path = filename;
    }

    QSettings mSetting(path, QSettings::IniFormat);
    mSetting.setIniCodec("UTF-8");
    return mSetting.value(key, defaultValue);
}

void util::setSetting(const QString& key, const QVariant& value, const QString& filename)
{
    QString path = filename;
    if (path.isEmpty())
    {
        path = util::appName();
    }
    path = QString("%1/%2.ini").arg(QApplication::applicationDirPath()).arg(path);
    QSettings mSetting(path, QSettings::IniFormat);
    mSetting.setIniCodec("UTF-8");
    mSetting.setValue(key, value);
}

int util::esleep(QEventLoop* loop, int msecs)
{
    QTimer* timer = new QTimer(loop);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [&]()
        {
            loop->exit(LoopTimeout);
        });
    timer->start(msecs);
    int ret = loop->exec();
    if (LoopTimeout == ret && msecs > 10)
    {
        qDebug() << "loop timeout";
    }
    return ret;
}

bool util::setProgramUnique(const QString& name)
{
    QLocalSocket localSocket;
    localSocket.connectToServer(name, QIODevice::WriteOnly);
    if (localSocket.waitForConnected())
    {
        char kk[1] = { '\0' };
        localSocket.write(kk, 3);
        localSocket.close();

        qWarning() << name << "is Existed.";
        return false;
    }
    localSocket.close();

    QLocalServer::removeServer(name);
    QLocalServer* localServer = new QLocalServer(qApp);
    localServer->listen(name);
    return true;
}

QString util::checkFile(const QString& filepath, bool isCover)
{
    QFileInfo finfo(filepath);
    QString suffix = finfo.completeSuffix();
    QString name = finfo.baseName();
    QString path = finfo.absolutePath();
    QDir dir;
    dir.mkpath(path);

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
        while (file.exists())
        {
            ++i;
            url = QString("%1/%2_%3.%4").arg(path).arg(name).arg(i).arg(suffix);
            file.setFileName(url);
        }
    }
    return url;
}

bool util::rmDir(const QString& path)
{
    if (path.isEmpty())
        return false;

    QDir dir(path);
    if (!dir.exists())
        return true;

    return dir.removeRecursively();
}

qint64 util::mSecsSinceEpoch()
{
    return QDateTime::currentMSecsSinceEpoch();
}

bool util::ensureDirExist(const QString& dirPath)
{
    QDir dir(dirPath);
    if (dir.exists())
    {
        return true;
    }
    return dir.mkpath(dirPath);
}

int util::screenWidth()
{
    return QApplication::desktop()->geometry().width();
}

int util::screenHeight()
{
    return QApplication::desktop()->geometry().height();
}

int util::scaleWidthByResolution(int width)
{
    int screenWidth = QApplication::desktop()->geometry().width();
    int newWidth = width / (float)1920 * screenWidth;
    return newWidth;
}

QSize util::newSize(QSize size)
{
    int screenWidth = QApplication::desktop()->geometry().width();
    int screenHeight = QApplication::desktop()->geometry().height();

    int newWidth = size.width() / (float)1920 * screenWidth;
    int newHeight = size.height() / (float)1080 * screenHeight;
    return QSize(newWidth, newHeight);
}

void util::resetMaxWidth(QLabel* label, int maxWidth)
{
    label->setMaximumWidth(util::scaleWidthByResolution(maxWidth));
}

void util::setTextWithEllipsis(QLabel* label, const QString& text)
{
    QFontMetrics fontWidth(label->font());
    QString elideNote = fontWidth.elidedText(text, Qt::ElideRight, label->maximumWidth());
    label->setText(elideNote);//显示省略号的字符串
    label->setToolTip(text);
}

QString util::logDir()
{
    return QString("%1/log").arg(QApplication::applicationDirPath());
}

QString util::appVersion()
{
    QString appVer = QString("%1").arg(QApplication::applicationVersion());
    appVer.chop(2);
    return appVer;
}

QString util::localIpv4()
{
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& interfaceItem : interfaceList) {
        if (interfaceItem.flags().testFlag(QNetworkInterface::IsUp) 
            && interfaceItem.flags().testFlag(QNetworkInterface::IsRunning) 
            && interfaceItem.flags().testFlag(QNetworkInterface::CanBroadcast) 
            && interfaceItem.flags().testFlag(QNetworkInterface::CanMulticast) 
            && !interfaceItem.flags().testFlag(QNetworkInterface::IsLoopBack) 
            && interfaceItem.hardwareAddress() != "00:50:56:C0:00:01" 
            && interfaceItem.hardwareAddress() != "00:50:56:C0:00:08") {
            QList<QNetworkAddressEntry> addressEntryList = interfaceItem.addressEntries();
            for (const QNetworkAddressEntry& addressEntryItem : addressEntryList) {
                if (addressEntryItem.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    return addressEntryItem.ip().toString();
                }
            }
        }
    }
    return QString("");
}

QMap<QString, QString> util::localIpv4Map() {
    QMap<QString, QString>     ret;
    QMap<QString, QStringList> nicIpMap;
#ifdef _WIN32
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD            dwRetVal = 0;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo      = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        qDebug() << "Error allocating memory needed to call GetAdaptersinfo";
        return ret;
    }
    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            qDebug() << "Error allocating memory needed to call GetAdaptersinfo";
            return ret;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            QString desc(pAdapter->Description);
            if (desc.contains("VMnet1") || desc.contains("VMnet8") || desc.contains("Hyper-V") ||
                desc.contains("Microsoft Wi-Fi Direct Virtual Adapter") ||
                desc.contains("Npcap Loopback Adapter")) {
                pAdapter = pAdapter->Next;
                continue;
            }
            qDebug() << "Adapter Desc:\t" << pAdapter->Description;

            QStringList     iplist;
            IP_ADDR_STRING* pAddr = &pAdapter->IpAddressList;
            while (pAddr) {
                QString ip(pAddr->IpAddress.String);
                if (!ip.contains("0.0.0.0") && !ip.startsWith("169.254")) {
                    qDebug() << "\tIP Address: \t" << pAddr->IpAddress.String;
                    iplist.append(ip);
                }
                pAddr = pAddr->Next;
            }
            if (!iplist.empty()) {
                nicIpMap.insert(desc, iplist);
            }

            pAdapter = pAdapter->Next;
        }
    } else {
        qDebug() << "GetAdaptersInfo failed with error:" << dwRetVal;
    }
    if (pAdapterInfo)
        free(pAdapterInfo);

    for (auto cit = nicIpMap.constBegin(); cit != nicIpMap.constEnd(); ++cit) {
        for (const auto& ip : cit.value()) {
            QString ipstr = QString("%1  (%2)").arg(cit.key(), ip);
            ret.insert(ipstr, ip);
        }
    }

#endif  // _WIN32

    return ret;
}

bool util::haveNvidiaGpu()
{
    IDXGIFactory* pFactory;
    IDXGIAdapter* pAdapter;
    std::vector <IDXGIAdapter*> vAdapters;            // 显卡           
    int iAdapterNum = 0; // 显卡的数量  

    // 创建一个DXGI工厂  
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

    if (FAILED(hr))
    {
        qWarning() << "CreateDXGIFactory failed";
        return false;
    }

    // 枚举适配器  
    while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        ++iAdapterNum;
    }

    for (size_t i = 0; i < vAdapters.size(); i++)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        vAdapters[i]->GetDesc(&adapterDesc);
        std::wstring wstrDesc(adapterDesc.Description);
        QString desc = QString::fromStdWString(wstrDesc);
        if (desc.contains("NVIDIA GeForce"))
        {
            return true;
        }
    }
    return false;
}


QString util::formatTime(int secs, const QString& format)
{
    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = (secs % 3600) % 60;

    return format.arg(h).arg(m).arg(s);
}

QStringList util::secToTime(int secs)
{
    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = (secs % 3600) % 60;

    QString hh = QString("%1").arg(h, 2, 10, QChar('0'));
    QString mh = QString("%1").arg(m, 2, 10, QChar('0'));
    QString sh = QString("%1").arg(s, 2, 10, QChar('0'));
    QStringList sl;
    sl.append(hh);
    sl.append(mh);
    sl.append(sh);

    return sl;
}

QString util::timestamp2String(qint64 ms)
{
    return QDateTime::fromMSecsSinceEpoch(ms).toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString util::QVariant2QString(const QVariant& var)
{
    return QJsonDocument::fromVariant(var).toJson(QJsonDocument::Compact);
}

QVariant util::QString2QVariant(const QString& s)
{
    return QJsonDocument::fromJson(s.toUtf8(), nullptr).toVariant();
}

QString util::logRtspUrl(const QString& url)
{
    QString ret;
    QStringList sl = url.split('@');
    if (2 != sl.length())
    {
        return ret;
    }
    QString ipPart = sl.last();
    QStringList sl2 = ipPart.split('/');
    if (sl2.empty())
    {
        return ret;
    }
    QString channel = sl2.last();
    QString ipPort = sl2.first();
    QStringList sl3 = ipPort.split(':');
    if (sl3.empty())
    {
        return ret;
    }
    QString ip = sl3.first();
    QStringList sl4 = ip.split('.');
    if (4 != sl4.length())
    {
        return ret;
    }
    return QString("%1-%2-%3").arg(sl4[2]).arg(sl4[3]).arg(channel);
}

QString util::logRtspError(const QString& url)
{
    QString op;
    if (url.endsWith('1'))
    {
        op = qstr("存储");
    }
    else if (url.endsWith('2'))
    {
        op = qstr("播放");
    }
    else
    {
        op = qstr("打开");
    }
    return qstr("摄像头%1%2失败").arg(util::logRtspUrl(url)).arg(op);
}

bool util::isDriveExist(const QString& drive)
{
    //QFileInfoList list = QDir::drives();
    foreach(const QFileInfo & fi, QDir::drives())
    {
        qDebug() << fi.filePath();
        if (fi.filePath() == drive)
        {
            return true;
        }
    }
    return false;
}

bool util::isFileExist(const QString& path)
{
    QFile file(path);
    return file.exists();
}

QStringList util::filePathListInDir(const QString& dirPath, QStringList filter)
{
    QDir dir(dirPath);
    if (!dir.exists())
    {
        return QStringList();
    }
    QStringList pathList;
    QFileInfoList list = dir.entryInfoList(filter, QDir::Files | QDir::Readable, QDir::Name);
    for each (QFileInfo fi in list)
    {
        pathList.append(fi.absoluteFilePath());
    }
    return pathList;
}

QString util::appName()
{
    QString name = QCoreApplication::applicationName();
    if (name.isEmpty())
    {
        name = APPNAME;
    }
    return name;
}

void util::setRetainSizeWhenHidden(QWidget* w, bool retain)
{
    QSizePolicy sp_retain = w->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    w->setSizePolicy(sp_retain);
}
