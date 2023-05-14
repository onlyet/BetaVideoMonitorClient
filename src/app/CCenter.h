#include "IFlowWidget.h"
#include <QVariant>

class QTcpSocket;
class SmallSeat;
class QWidget;
class CSmallScreen;

class CCenter : public IFlowWidget
{
    Q_OBJECT

public:
    explicit CCenter(QWidget *parent = nullptr);
    ~CCenter() Q_DECL_OVERRIDE;

    // 创建座位
    void createSeat();
    // 初始化一页的座位
    void init1PageSeat();
    void init1PageSeat(int min, int max);

    // ui初始化, 窗口适配
    void init();

    void loadData();

    // 暂停/继续所有播放窗口
    void pause(bool isPause);

    // 打开所有播放器
    void openAll();

    // 关闭所有播放器
    void closeAll();

    // 打开所有播放器并开始录制
    void allPeopleStart();

    // 关闭所有播放器并停止录制
    void allPeopleStop();

    void setTransparentLabelVisable(bool visable, int delayMs = 0);
    void switch24Screen();
    void switch28Screen();
    void switch12Screen();
    void switch8Screen();

public slots:

    void currentPageChanged(int page);

protected slots:
    // 大屏查看
    void onFocus(QVariantMap data);

signals:
    // 打开大屏
    void focus(QVariantMap data);

private:
    int                  m_scrollStep;  // 垂直滚动步进
    SmallSeat*           m_smallSeat = nullptr;
    int                  m_maxSeatNum;      // 座位总数
    int                  m_1PageSeatNum;    // 一页展示的座位数
    int                  m_row;             // 一页的行数
    int                  m_column{4};       // 一页的列数
    QList<CSmallScreen*> m_seatWidgetList;  // 28个CSeat
};

