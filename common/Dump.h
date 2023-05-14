#ifndef DUMP_H
#define DUMP_H

#include <log.h>
#include <util.h>
#include <GlobalSignalSlot.h>
#include <CMessageBox.h>
#include <Toast.h>

#include <tchar.h>
#include <Windows.h>
#include <DbgHelp.h>

#include <QSettings>
#include <QDebug>


int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
    GSignalSlot.notifyCrash();
    qInfo().noquote() << QStringLiteral("�ͻ��˱�����");

    // ���庯��ָ��
    typedef BOOL(WINAPI* MiniDumpWriteDumpT)(
        HANDLE,
        DWORD,
        HANDLE,
        MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION,
        PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION
        );
    // �� "DbgHelp.dll" ���л�ȡ "MiniDumpWriteDump" ����
    MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelp = LoadLibrary(_T("DbgHelp.dll"));
    if (NULL == hDbgHelp)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    if (NULL == pfnMiniDumpWriteDump)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // ���� dmp �ļ���
    TCHAR szFileName[MAX_PATH] = { 0 };
    LPWSTR szVersion = const_cast<LPWSTR>(L"DumpFile");
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);
    wsprintf(szFileName, _T("%s-%04d-%02d-%02d-%02d-%02d-%02d.dmp"),
        szVersion, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond);
    HANDLE hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    if (INVALID_HANDLE_VALUE == hDumpFile)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // д�� dmp �ļ�
    MINIDUMP_EXCEPTION_INFORMATION expParam;
    expParam.ThreadId = GetCurrentThreadId();
    expParam.ExceptionPointers = pExceptionPointers;
    expParam.ClientPointers = FALSE;
    pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
    // �ͷ��ļ�
    CloseHandle(hDumpFile);
    FreeLibrary(hDbgHelp);

#ifdef _WIN32
    //MessageBox(NULL, TEXT("�ͻ��˱����ˣ�����ϵ����֧����Ա"), TEXT("������ʾ"), 0);
#endif // _WIN32
    Toast::showMessage(qstr("�ͻ��˱����ˣ�����ϵ����֧����Ա"));

    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
    // ������һЩ�쳣�Ĺ��˻���ʾ
    if (IsDebuggerPresent())
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return GenerateMiniDump(lpExceptionInfo);
}

// �˺���һ���ɹ����ã�֮��� SetUnhandledExceptionFilter �ĵ��ý���Ч
void DisableSetUnhandledExceptionFilter()
{
    void* addr = (void*)GetProcAddress(LoadLibrary(L"kernel32.dll"),
        "SetUnhandledExceptionFilter");

    if (addr)
    {
        unsigned char code[16];
        int size = 0;

        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag, dwTempFlag;
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
    }
}

void DumpInit()
{
    QString path = QString("%1/%2.ini").arg(QCoreApplication::applicationDirPath()).arg(util::appName());
    QSettings mSetting(path, QSettings::IniFormat);
    mSetting.setIniCodec("UTF-8");

    bool useVectoredExceptionHandler = mSetting.value(UseVectoredExceptionHandler, false).toBool();
    if (useVectoredExceptionHandler)
    {
        AddVectoredExceptionHandler(1, ExceptionFilter);
    }
    else
    {
        SetUnhandledExceptionFilter(ExceptionFilter);
    }
}

#endif // DUMP_H
