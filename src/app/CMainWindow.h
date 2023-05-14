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

	// �Ƿ���ʾ�ؼ���ʹ�ܿؼ�
    void showInfo(bool ifshow);

    // ����ʱ���ǩ
    void updateTimeLabel();
    void updateTime();

    void setUiBlank();

    void startRecord();
    void endRecord();

protected slots:
    void onBtnClose();
    void onBtnCmd();
    void onBtnSetting();
    // �������ͷ�����б��
    void onCBoxCamerasCurrentIndexChanged(int idx);

    void onTimerUpdateServerTime();

    // �򿪴���������С����������ͣ����
    void onFocus(QVariantMap data);
    // �Ӵ�������
    void onBack();

    void currentPageChanged(int page);

    void onIpcParamsError(const QString &msg);
    void onCrashed();

private:
    Ui::CMainWindow*     ui;
    bool                 m_recordStarted   = false;    // �Ƿ�����¼��
    QTimer*              m_updateTimeTimer = nullptr;  // ����ʱ�䶨ʱ��
    LoadingLabel*        m_loadingLabel    = nullptr;  // ����gif
    QMovie*              m_movie           = nullptr;
    bool                 m_useShade;  // �Ƿ�ʹ������
    bool                 m_showPopup        = true;
    QLineEdit*           m_cameraEdit       = nullptr;
    QAction*             m_recordDirAct     = nullptr;  // ��¼����ƵĿ¼
    QAction*             m_logAct           = nullptr;  // ����־Ŀ¼
    SimplePageNavigator* m_pageNavigator    = nullptr;  // �ײ�ҳ����
    int                  m_concatProcessNum = 0;        // �ϲ�������
};

#endif // CMAINWINDOW_H
