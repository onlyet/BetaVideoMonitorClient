#include <QFrame>

namespace Ui {
class CBigScreen;
}

class CBigScreen : public QFrame
{
    Q_OBJECT

public:
    explicit CBigScreen(QWidget *parent = nullptr);
    ~CBigScreen() Q_DECL_OVERRIDE;

    void init(QVariantMap data);
    void setName(const QString &name);

    void startPlay() const;
    void stopPlay() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void on_act_back_triggered();

signals:
    void back();

private:
    Ui::CBigScreen *ui;

    int         m_seat;         // 座位号
    QString     m_name;         // 名字
};

