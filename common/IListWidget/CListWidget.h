#pragma once

#include <IListWidget.h>

#include <QWidget>
namespace Ui { class CListWidget; };

class CListWidget : public IListWidget
{
	Q_OBJECT

public:
	CListWidget(QWidget *parent = Q_NULLPTR);
	~CListWidget();


protected:
	void enterEvent(QEvent* ev) override;
	void leaveEvent(QEvent* ev) override;

private:
	Ui::CListWidget *ui;
};
