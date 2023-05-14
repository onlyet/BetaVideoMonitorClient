#include "CMessageBox.h"

#include "CConfirmBox.h"
#include "CInfoBox.h"

bool CMessageBox::confirm(const QString& text, QWidget* parent)
{
    CConfirmBox box(text, parent);
    return box.exec() == QDialog::Accepted;
}

void CMessageBox::info(const QString& text, QWidget* parent)
{
    CInfoBox box(text, parent);
    box.exec();
}
