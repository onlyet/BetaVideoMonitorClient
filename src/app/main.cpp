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

    // ��ʼ����־
    LogInit(util::appName(), a.applicationVersion());

    // ��ʼ������ģ��
	DumpInit();

    //ʹ����Ψһ
	if (!util::setProgramUnique(util::appName()))
	{
        qWarning() << qstr("�Ѵ���һ���ͻ���ʵ��");
		return -1;
	}

	// ��ʼ��ȫ������
	if (!CONFIG.init())
	{
		return -1;
	}

    // ������
    CMainWindow w;
    w.showFullScreen();

    // �˳�ǰ���������������taskkillɱ�����̻ᴥ������
    QObject::connect(&a, &QApplication::aboutToQuit, [&w](){
        w.aboutToQuit();
    });
    int ret = a.exec();
    return ret;
}
