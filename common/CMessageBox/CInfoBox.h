#ifndef CINFOBOX_H
#define CINFOBOX_H

#include "IMessageBox.h"

namespace Ui {
    class CInfoBox;
}

/*! @brief 提示窗口 @see CMessageBox::info */
class CInfoBox : public IMessageBox
{
    Q_OBJECT
public:
    explicit CInfoBox(QWidget* parent = Q_NULLPTR);
    explicit CInfoBox(const QString& text, QWidget* parent = nullptr);
    ~CInfoBox();

    /*! @brief 设置提示文本 */
    void setText(const QString& text);

private:
    Ui::CInfoBox* ui;
};

#endif // CINFOBOX_H
