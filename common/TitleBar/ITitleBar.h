#ifndef ITITLEBAR_H
#define ITITLEBAR_H

#include <QFrame>

/*! @brief ���϶������� */
class ITitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit ITitleBar(QWidget* parent = Q_NULLPTR);
    ~ITitleBar() override;

    /*! @brief �����϶�Ŀ�괰�� */
    void setWidget(QWidget* wid);

protected:
    /*! @brief ����¼�, �����ƶ����� */
    void mouseMoveEvent(QMouseEvent *event) override;
    /*! @brief ����¼�, ������ʼ�ƶ����� */
    void mousePressEvent(QMouseEvent *event) override;
    /*! @brief ����¼�, ���������ƶ����� */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QWidget* m_pMoveWidget; ///< �ƶ�Ŀ�괰��
    bool m_isMousePressed;  ///< �Ƿ������, �����ƶ�����
    QPoint m_mousePoint;    ///< ��¼���λ��, �����ƶ�����
};

#endif // ITITLEBAR_H
