#pragma once

#include <QWidget>
#include <QVariant>

namespace Ui { class IFlowWidget; };

class FlowLayout;

class IFlowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IFlowWidget(QWidget *parent = Q_NULLPTR);
    ~IFlowWidget() Q_DECL_OVERRIDE;

    void setRow(int row);
    //void setColumn(int col);
    int row();
    int column();
    int count();

    void addItems(const QList<QWidget*> &wids);

    void clear();
    // 重置UI为空白，但是不删除item
    void clearUI();
    // 重置UI为空白，删除所有item
    void deleteAll();

    QWidget *item(int idx);
    QWidget *item(int row, int col);

    void setMargin(int left, int top, int right, int bottom);

    int hSpacing();
    int vSpacing();

    void reset(const QVariantMap &spaceInfo);

protected:
    QList<QWidget *> items() const;

private:
    bool                m_byRow;        // 是否以行数为基准
    int                 m_row;          // 行数
    int                 m_col;          // 列数

    QList<QWidget *>    m_items;        // item列表
    FlowLayout          *m_pLayout;     // 布局
};

