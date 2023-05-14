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

    /*! @brief ��ffmpeg��AVRationalת��Ϊ���õ�fps */
    double r2d(AVRational r);

    /*! @brief ����ָ���16�����ַ�����ʽ */
    QString getPtrString(void *ptr);

    /**
     * @brief ��������ļ�·��, ���غ���������ļ�·��. �������·���������򴴽�.
     * @param[in] filepath  �ļ�·��
     * @param[in] isCover   �Ƿ�ɾ��ͬ���ļ�. Ϊfalseʱ, �����ļ������������.
     * */
    QString checkFile(const QString &filepath, bool isCover);
};

