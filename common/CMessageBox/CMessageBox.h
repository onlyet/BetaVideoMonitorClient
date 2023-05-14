#ifndef CMESSAGEBOX_H
#define CMESSAGEBOX_H

#include <QString>

class QWidget;

/*! @brief 消息弹窗 */
namespace CMessageBox {
    /*! @brief 弹出确认窗口 @return 返回是否确认 */
    bool confirm(const QString& text, QWidget* parent = nullptr);
    /*! @brief 弹出提示窗口 */
    void info(const QString& text, QWidget* parent = nullptr);
}

#endif // CMESSAGEBOX_H
