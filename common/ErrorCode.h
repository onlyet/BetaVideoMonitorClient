#pragma once

#define ErrorCode_IPCOk                     0           // ����
#define ErrorCode_IPCOpenFailed             1 << 0      // �򲻿�
#define ErrorCode_IPCWidthOrHeightIs0       1 << 1      // ����Ϊ0
#define ErrorCode_IPCResolutionError        1 << 5      // �ֱ��ʴ���
#define ErrorCode_IPCCodecTypeError         1 << 6      // �������ʹ���
