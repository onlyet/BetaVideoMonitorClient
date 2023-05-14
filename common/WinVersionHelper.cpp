﻿#include "WinVersionHelper.h"
#include <Windows.h>


CWinVersionHelper& CWinVersionHelper::instance()
{
    static CWinVersionHelper s_instance;
    return s_instance;
}

CWinVersionHelper::CWinVersionHelper()
{
    DWORD dwMajorVer{}, dwMinorVer{}, dwBuildNumber{};
    HMODULE hModNtdll{};
    if (hModNtdll = ::LoadLibraryW(L"ntdll.dll"))
    {
        typedef void (WINAPI* pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);
        pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
        pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
        if (pfRtlGetNtVersionNumbers)
        {
            pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
            dwBuildNumber &= 0x0ffff;
        }
        ::FreeLibrary(hModNtdll);
        hModNtdll = NULL;
    }
    m_major_version = dwMajorVer;
    m_minor_version = dwMinorVer;
    m_build_number = dwBuildNumber;
}


CWinVersionHelper::~CWinVersionHelper()
{
}

bool CWinVersionHelper::IsWindows10FallCreatorOrLater() const
{
    if (m_major_version > 10)
        return true;
    else if (m_major_version == 10 && m_minor_version > 0)
        return true;
    else if (m_major_version == 10 && m_minor_version == 0 && m_build_number >= 16299)
        return true;
    else return false;
}

bool CWinVersionHelper::IsWindows7() const
{
    return (m_major_version == 6 && m_minor_version == 1);
}

bool CWinVersionHelper::IsWindows8Or8point1() const
{
    return (m_major_version == 6 && m_minor_version > 1);
}

bool CWinVersionHelper::IsWindows8OrLater() const
{
    if (m_major_version > 6)
        return true;
    else if (m_major_version == 6 && m_minor_version >= 2)
        return true;
    else return false;
}

bool CWinVersionHelper::IsWindows10OrLater() const
{
    return m_major_version >= 10;
}
