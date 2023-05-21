#include "CMainWindow.h"
#include "log.h"
#include "util.h"
#include "common.h"

#include <GlobalConfig.h>
#include <Dump.h>
#include <CMessageBox.h>

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化日志
    LogInit(util::appName(), a.applicationVersion());

    // 初始化崩溃模块
	DumpInit();

    //使程序唯一
	if (!util::setProgramUnique(util::appName()))
	{
        qWarning() << "已存在一个客户端实例";
		return -1;
	}

	// 初始化全局配置
	if (!CONFIG.init())
	{
		return -1;
	}

    // 主窗口
    CMainWindow w;
    w.showFullScreen();

    // 退出前清理，任务管理器或taskkill杀掉进程会触发这里
    QObject::connect(&a, &QApplication::aboutToQuit, [&w](){
        w.aboutToQuit();
    });
    int ret = a.exec();
    return ret;
}
