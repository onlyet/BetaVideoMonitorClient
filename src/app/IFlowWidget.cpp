#include "IFlowWidget.h"
#include "ui_IFlowWidget.h"

#include "IFlowWidget.h"
#include "ui_IFlowWidget.h"
#include <flowlayout.h>

#include <QDebug>
#include <QTimer>

IFlowWidget::IFlowWidget(QWidget *parent)
    : QWidget(parent)
    , m_byRow(false)
    , m_row(1)
    , m_col(1)
{
    m_pLayout = new FlowLayout(this);
}

IFlowWidget::~IFlowWidget()
{
}

void IFlowWidget::setRow(int row)
{
    m_byRow = true;
    if (row <= 0) row = 1;

    int col = (count() - 1) / row + 1;
    m_row = row;
    m_col = col;
}

//void IFlowWidget::setColumn(int col)
//{
//    m_byRow = false;
//    if (col <= 0) col = 1;
//
//    int row = (count() - 1) / col + 1;
//    m_row = row;
//    m_col = col;
//
//    reset();
//}

int IFlowWidget::row()
{
    return m_row;
}

int IFlowWidget::column()
{
    return m_col;
}

int IFlowWidget::count()
{
    return m_items.size();
}

void IFlowWidget::addItems(const QList<QWidget *> &wids)
{
    if (wids.isEmpty()) return;
    m_items.append(wids);
}

void IFlowWidget::clear()
{
    m_items.clear();
}

void IFlowWidget::clearUI()
{
    while (!m_items.isEmpty())
    {
        QWidget* w = m_items.takeFirst();
        w->setParent(nullptr);
    }
}

void IFlowWidget::deleteAll()
{
    while (!m_items.isEmpty())
    {
        QWidget* w = m_items.takeFirst();
        w->setParent(nullptr);
        delete w;
    }
}

QWidget *IFlowWidget::item(int idx)
{
    if (idx >= m_items.size() || idx < 0) return Q_NULLPTR;
    return m_items[idx];
}

QWidget *IFlowWidget::item(int row, int col)
{
    if (row < 1 || col < 1) return Q_NULLPTR;
    if (row > m_row || col > m_col) return Q_NULLPTR;

    int idx = (row - 1) * m_col + (col - 1);
    return m_items[idx];
}

void IFlowWidget::setMargin(int left, int top, int right, int bottom)
{
    m_pLayout->setContentsMargins(left, top, right, bottom);
}

int IFlowWidget::hSpacing()
{
    return m_pLayout->horizontalSpacing();
}

int IFlowWidget::vSpacing()
{
    return m_pLayout->verticalSpacing();
}

void IFlowWidget::reset(const QVariantMap &spaceInfo)
{
    int hSpace = spaceInfo["hSpace"].toInt();
    int vSpace = spaceInfo["vSpace"].toInt();
    int hMargin = spaceInfo["hMargin"].toInt();
    int vMargin = spaceInfo["vMargin"].toInt();
    m_pLayout->setHSpace(hSpace);
    m_pLayout->setVSpace(vSpace);
    m_pLayout->setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    for (const auto &w : m_items)
    {
        m_pLayout->addWidget(w);
    }
}

QList<QWidget*> IFlowWidget::items() const
{
    return m_items;
}
