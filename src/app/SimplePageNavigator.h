#ifndef SIMPLEPAGENAVIGATOR_H
#define SIMPLEPAGENAVIGATOR_H

#include <QWidget>
#include <QList>

class QLabel;
class QEvent;
class QLineEdit;

namespace Ui
{
    class SimplePageNavigator;
}

class SimplePageNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit SimplePageNavigator(int blockSize = 3, QWidget *parent = nullptr);
    ~SimplePageNavigator();

    int getBlockSize() const;
    int getMaxPage() const;
    int getCurrentPage() const;
    int aPageDataNum() const;

    // 其他组件只需要调用这两个函数即可
    void setMaxPage(int page);   // 当总页数改变时调用
    void setCurrentPage(int page, bool signalEmitted = false); // 修改当前页时调用
    void setDataNum(int num);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);
    void updatePaging();

signals:
    void currentPageChanged(int page);
    void aPageDataNumChanged(int aPageDataNum);

private:
    Ui::SimplePageNavigator         *ui;
    int                             m_blockSize;
    int                             m_dataNum = 0;          // 总数据量
    int                             m_aPageDataNum = 0;     // 每页数据量
    int                             m_maxPage;              // 一共多少页
    int                             m_currentPage;
    QList<QLabel *>                 *m_pageLabels;

    void setBlockSize(int blockSize);
    void updatePageLabels();
    void initialize();

};

#endif // SIMPLEPAGENAVIGATOR_H
