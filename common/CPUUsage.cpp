#include "CPUUsage.h"
#include <iostream>
#include "WinVersionHelper.h"

using namespace std;

void CCPUUsage::SetUseCPUTimes(bool use_get_system_times)
{
    if (m_use_get_system_times != use_get_system_times)
    {
        m_use_get_system_times = use_get_system_times;
        m_first_get_CPU_utility = true;
    }
}

int CCPUUsage::GetCPUUsage()
{
    //if (m_use_get_system_times)
    //	return GetCPUUsageByGetSystemTimes();
    //else
    return GetCPUUsageByPdh();
}

#if 0
int CCPUUsage::GetCPUUsageByGetSystemTimes()
{
    int cpu_usage{};
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    __int64 idle = CCommon::CompareFileTime2(m_preidleTime, idleTime);
    __int64 kernel = CCommon::CompareFileTime2(m_prekernelTime, kernelTime);
    __int64 user = CCommon::CompareFileTime2(m_preuserTime, userTime);

    if (kernel + user == 0)
    {
        cpu_usage = 0;
    }
    else
    {
        //���ܵ�ʱ��-����ʱ�䣩/�ܵ�ʱ��=ռ��cpu��ʱ�����ʹ����
        cpu_usage = static_cast<int>(abs((kernel + user - idle) * 100 / (kernel + user)));
    }
    m_preidleTime = idleTime;
    m_prekernelTime = kernelTime;
    m_preuserTime = userTime;

    return cpu_usage;
}
#endif


int CCPUUsage::GetCPUUsageByPdh()
{
    int cpu_usage{};
    HQUERY hQuery;
    HCOUNTER hCounter;
    DWORD counterType;
    PDH_RAW_COUNTER rawData;

    PdhOpenQuery(NULL, 0, &hQuery);//��ʼ��ѯ
    const wchar_t* query_str{};
    if (g_winVerHelper.GetMajorVersion() >= 10)
        query_str = L"\\Processor Information(_Total)\\% Processor Utility";
    else
        query_str = L"\\Processor Information(_Total)\\% Processor Time";
    PdhAddCounter(hQuery, query_str, NULL, &hCounter);
    PdhCollectQueryData(hQuery);
    PdhGetRawCounterValue(hCounter, &counterType, &rawData);

    if (m_first_get_CPU_utility) {//��Ҫ����������ݲ��ܼ���CPUʹ����
        cpu_usage = 0;
        m_first_get_CPU_utility = false;
    }
    else {
        PDH_FMT_COUNTERVALUE fmtValue;
        PdhCalculateCounterFromRawValue(hCounter, PDH_FMT_DOUBLE, &rawData, &m_last_rawData, &fmtValue);//����ʹ����
        cpu_usage = fmtValue.doubleValue;//��������
        if (cpu_usage > 100)
            cpu_usage = 100;
    }
    m_last_rawData = rawData;//������һ������
    PdhCloseQuery(hQuery);//�رղ�ѯ
    return cpu_usage;
}
