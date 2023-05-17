#pragma once

#define ErrorCode_IPCOk                     0           // 正常
#define ErrorCode_IPCOpenFailed             1 << 0      // 打不开
#define ErrorCode_IPCWidthOrHeightIs0       1 << 1      // 宽或高为0
#define ErrorCode_IPCResolutionError        1 << 5      // 分辨率错误
#define ErrorCode_IPCCodecTypeError         1 << 6      // 编码类型错误
