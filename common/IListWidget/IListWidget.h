#ifndef ILISTWIDGET_H
#define ILISTWIDGET_H

#include <QListWidget>

namespace Ui {
class IListWidget;
}

class QEvent;

class IListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit IListWidget(QWidget *parent = nullptr);
    ~IListWidget();

private:
    Ui::IListWidget *ui;
};

#endif // ILISTWIDGET_H
