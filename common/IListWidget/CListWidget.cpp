#include "CListWidget.h"
#include "ui_CListWidget.h"

CListWidget::CListWidget(QWidget *parent)
	: IListWidget(parent)
{
	ui = new Ui::CListWidget();
	ui->setupUi(this);

	// 避免最后一行空白
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

CListWidget::~CListWidget()
{
	delete ui;
}


void CListWidget::enterEvent(QEvent* ev)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	QWidget::enterEvent(ev);
}

void CListWidget::leaveEvent(QEvent* ev)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QWidget::enterEvent(ev);
}
