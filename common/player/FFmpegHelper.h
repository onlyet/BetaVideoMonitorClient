#include <QString>

extern "C"
{
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
    #include "libavdevice/avdevice.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/time.h"
}

namespace FFmpegHelper
{
    void init_ffmpeg();
    QString parseErr(int errNum);

    /*! @brief 将ffmpeg的AVRational转化为可用的fps */
    double r2d(AVRational r);

    /*! @brief 返回指针的16进制字符串形式 */
    QString getPtrString(void *ptr);

    /**
     * @brief 检查完整文件路径, 返回合理的完整文件路径. 如果所属路径不存在则创建.
     * @param[in] filepath  文件路径
     * @param[in] isCover   是否删除同名文件. 为false时, 会在文件名后增加序号.
     * */
    QString checkFile(const QString &filepath, bool isCover);
};

