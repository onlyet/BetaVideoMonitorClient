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

    // ������λ
    void createSeat();
    // ��ʼ��һҳ����λ
    void init1PageSeat();
    void init1PageSeat(int min, int max);

    // ui��ʼ��, ��������
    void init();

    void loadData();

    // ��ͣ/�������в��Ŵ���
    void pause(bool isPause);

    // �����в�����
    void openAll();

    // �ر����в�����
    void closeAll();

    // �����в���������ʼ¼��
    void allPeopleStart();

    // �ر����в�������ֹͣ¼��
    void allPeopleStop();

    void setTransparentLabelVisable(bool visable, int delayMs = 0);
    void switch24Screen();
    void switch28Screen();
    void switch12Screen();
    void switch8Screen();

public slots:

    void currentPageChanged(int page);

protected slots:
    // �����鿴
    void onFocus(QVariantMap data);

signals:
    // �򿪴���
    void focus(QVariantMap data);

private:
    int                  m_scrollStep;  // ��ֱ��������
    SmallSeat*           m_smallSeat = nullptr;
    int                  m_maxSeatNum;      // ��λ����
    int                  m_1PageSeatNum;    // һҳչʾ����λ��
    int                  m_row;             // һҳ������
    int                  m_column{4};       // һҳ������
    QList<CSmallScreen*> m_seatWidgetList;  // 28��CSeat
};

