#ifndef CCONFIRMBOX_H
#define CCONFIRMBOX_H

#include "IMessageBox.h"

namespace Ui {
    class CConfirmBox;
}

/*! @brief 确认窗口 @see CMessageBox::comfirm */
class CConfirmBox : public IMessageBox
    //class CConfirmBox : public QDialog
{
    Q_OBJECT

public:
    explicit CConfirmBox(QWidget* parent = nullptr);
    explicit CConfirmBox(const QString& text, QWidget* parent = nullptr);
    ~CConfirmBox();

    /*! @brief 设置提示文本 */
    void setText(const QString& text);

private:
    Ui::CConfirmBox* ui;
};

#endif // CCONFIRMBOX_H
