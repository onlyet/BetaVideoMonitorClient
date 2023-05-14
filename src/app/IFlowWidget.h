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
    // ����UIΪ�հף����ǲ�ɾ��item
    void clearUI();
    // ����UIΪ�հף�ɾ������item
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
    bool                m_byRow;        // �Ƿ�������Ϊ��׼
    int                 m_row;          // ����
    int                 m_col;          // ����

    QList<QWidget *>    m_items;        // item�б�
    FlowLayout          *m_pLayout;     // ����
};

