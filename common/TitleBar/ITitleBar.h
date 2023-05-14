#ifndef ITITLEBAR_H
#define ITITLEBAR_H

#include <QFrame>

/*! @brief 可拖动标题栏 */
class ITitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit ITitleBar(QWidget* parent = Q_NULLPTR);
    ~ITitleBar() override;

    /*! @brief 设置拖动目标窗口 */
    void setWidget(QWidget* wid);

protected:
    /*! @brief 鼠标事件, 用来移动窗口 */
    void mouseMoveEvent(QMouseEvent *event) override;
    /*! @brief 鼠标事件, 用来开始移动窗口 */
    void mousePressEvent(QMouseEvent *event) override;
    /*! @brief 鼠标事件, 用来结束移动窗口 */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QWidget* m_pMoveWidget; ///< 移动目标窗口
    bool m_isMousePressed;  ///< 是否按下鼠标, 用来移动窗口
    QPoint m_mousePoint;    ///< 记录鼠标位置, 用来移动窗口
};

#endif // ITITLEBAR_H
