#ifndef IMESSAGEBOX_H
#define IMESSAGEBOX_H

#include <QDialog>

/*! @brief ��Ϣ���� */
class IMessageBox : public QDialog
{
public:
    explicit IMessageBox(QWidget* parent = Q_NULLPTR);
    ~IMessageBox() Q_DECL_OVERRIDE;

protected:
    /*! @brief ����¼�, �����ƶ����� */
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    /*! @brief ����¼�, ������ʼ�ƶ����� */
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    /*! @brief ����¼�, ���������ƶ����� */
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    bool m_isMousePressed;   ///< �Ƿ������, �����ƶ�����
    QPoint m_mousePoint;    ///< ��¼���λ��, �����ƶ�����
};

#endif // IMESSAGEBOX_H
