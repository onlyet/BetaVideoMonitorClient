#ifndef CSEAT_H
#define CSEAT_H

#include <QFrame>
#include <QVariantMap>

namespace Ui {
class CSmallScreen;
}

class QAction;
class CTransparentLabel;

class CSmallScreen : public QFrame
{
    Q_OBJECT

public:
    explicit CSmallScreen(QWidget *parent = nullptr, bool isPlaceHolder = false);
    ~CSmallScreen() Q_DECL_OVERRIDE;

    int serialNum() const;
    void setSerialNum(int serialNum);

    void setSeatInfo(int serialNum);
    int seat() const { return m_seat; }

    QString name() const { return m_name; }

    void setLeftUrl(const QString &url);
    void setRightUrl(const QString &url);

    const QString &leftUrl() const;
    const QString &rightUrl() const;

    void startPlay();
    void stopPlay();
    void pause(bool isPause) const;

    void startRecord();
    void stopRecord();

    static QSize getPerferSize(QSize widgetSize, int row, int column, int xSpace, int ySpace, int &hMargin, int vMargin);

    void setSize(const QSize & s, const QVariantMap &spaceInfo);

    void setTransparentLabelVisable(bool visable);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

    void setInfo(const QString &name, const QString &title);

    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void initUi();

    void initTransparentLabel();

public slots:
    void setTransparentLabel(bool visable);

protected slots:
    void on_act_focus_triggered();
    void on_act_open_triggered();
    void on_act_close_triggered();

signals:
    void focus(QVariantMap data);
    void showAllTitle();

private:
    Ui::CSmallScreen*             ui;
    int                    m_seat;           // 座位号，可任意调整(1-28)
    int                    m_serialNum;      // 序号，实际物理位置 (1-28)
    QString                m_name;           
    QString                m_title;          
    QString                m_leftPlayUrl;    // 左摄像头子码流
    QString                m_leftSaveUrl;    // 左摄像头主码流
    QString                m_rightPlayUrl;   // 右摄像头子码流
    QString                m_rightSaveUrl;   // 右摄像头主码流
    QString                m_leftSavePath;   // 左侧视频保存路径
    QString                m_rightSavePath;  // 右侧视频保存路径
    bool                   m_recordStarted            = false;
    CTransparentLabel*     m_serialNumLabel           = nullptr;
};


#endif // CSEAT_H
