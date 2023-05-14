#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include <QVariantMap>
#include <QAbstractSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class CMainWindow; }
QT_END_NAMESPACE

class QProcess;
class QMovie;
class LoadingLabel;
class QTime;
class QAction;
class QLineEdit;
class QLabel;
class SimplePageNavigator;
class QNetworkReply;

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow() Q_DECL_OVERRIDE;

    void aboutToQuit();

protected:
    void initQAction();
    void initQComboBox();

    void initUi();
    void updateUi();
    void newLoadingShade();
    void showLoadingShade();
    void hideLoadingShade();

    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *ev) override;

    void openAllVideos();
    void closeAllVideos();

    void switch24Screen();
    void switch28Screen();
    void switch12Screen();
    void switch8Screen();

    void refreshBtnCmd();

	// 是否显示控件和使能控件
    void showInfo(bool ifshow);

    // 更新时间标签
    void updateTimeLabel();
    void updateTime();

    void setUiBlank();

    void startRecord();
    void endRecord();

protected slots:
    void onBtnClose();
    void onBtnCmd();
    void onBtnSetting();
    // 点击摄像头下拉列表框
    void onCBoxCamerasCurrentIndexChanged(int idx);

    void onTimerUpdateServerTime();

    // 打开大屏，所有小屏播放器暂停播放
    void onFocus(QVariantMap data);
    // 从大屏返回
    void onBack();

    void currentPageChanged(int page);

    void onIpcParamsError(const QString &msg);
    void onCrashed();

private:
    Ui::CMainWindow*     ui;
    bool                 m_recordStarted   = false;    // 是否正在录制
    QTimer*              m_updateTimeTimer = nullptr;  // 更新时间定时器
    LoadingLabel*        m_loadingLabel    = nullptr;  // 加载gif
    QMovie*              m_movie           = nullptr;
    bool                 m_useShade;  // 是否使用遮罩
    bool                 m_showPopup        = true;
    QLineEdit*           m_cameraEdit       = nullptr;
    QAction*             m_recordDirAct     = nullptr;  // 打开录制视频目录
    QAction*             m_logAct           = nullptr;  // 打开日志目录
    SimplePageNavigator* m_pageNavigator    = nullptr;  // 底部页导航
    int                  m_concatProcessNum = 0;        // 合并进程数
};

#endif // CMAINWINDOW_H
