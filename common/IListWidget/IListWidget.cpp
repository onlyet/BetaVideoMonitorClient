#include "IListWidget.h"
#include "ui_IListWidget.h"

#include <QScrollBar>

IListWidget::IListWidget(QWidget *parent) :
    QListWidget(parent),
    ui(new Ui::IListWidget)
{
    ui->setupUi(this);

    setFrameShape(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setDefaultDropAction(Qt::IgnoreAction);
    setDragEnabled(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setSingleStep(30);
    horizontalScrollBar()->setSingleStep(30);
    setContentsMargins(0, 0, 0, 0);


    // 背景透明则需要在外层QFrame设置背景色
    verticalScrollBar()->setStyleSheet("QScrollBar:vertical{background:transparent;width:6px;}"\
        "QScrollBar::handle:vertical{background:rgb(153,153,153);height:30px;}"\
        "QScrollBar::sub-line:vertical{background:transparent;}"\
        "QScrollBar::add-line:vertical{background:transparent;}"\
        "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:transparent;}");
}

IListWidget::~IListWidget()
{
    delete ui;
}

