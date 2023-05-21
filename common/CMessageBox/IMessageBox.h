#ifndef IMESSAGEBOX_H
#define IMESSAGEBOX_H

#include <QDialog>

/*! @brief 消息弹窗 */
class IMessageBox : public QDialog
{
public:
    explicit IMessageBox(QWidget* parent = Q_NULLPTR);
    ~IMessageBox() Q_DECL_OVERRIDE;

protected:
    /*! @brief 鼠标事件, 用来移动窗口 */
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    /*! @brief 鼠标事件, 用来开始移动窗口 */
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    /*! @brief 鼠标事件, 用来结束移动窗口 */
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    bool m_isMousePressed;   ///< 是否按下鼠标, 用来移动窗口
    QPoint m_mousePoint;    ///< 记录鼠标位置, 用来移动窗口
};

#endif // IMESSAGEBOX_H
