#include "CMainWindow.h"
#include "ui_CMainWindow.h"
#include "CMessageBox.h"
#include "util.h"
#include "LoadingLabel.h"
#include <GlobalConfig.h>
#include "GlobalSignalSlot.h"
#include "SimplePageNavigator.h"

#include <QMediaPlayer>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QProcess>
#include <windows.h>
#include <string>
#include <QMovie>
#include <QPainter>
#include <QStorageInfo>
#include <QListWidget>
#include <QLineEdit>
#include <QDesktopServices>
#include <QtConcurrent>
#include <QLocalSocket>
#include <QSCreen>

using namespace std;

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow)
    , m_loadingLabel(nullptr)
    , m_useShade(false)
{
    connect(QApplication::primaryScreen(), &QScreen::geometryChanged, [this]() {
        onCBoxCamerasCurrentIndexChanged(ui->cbox_camera->currentIndex());
    });
    connect(QApplication::primaryScreen(), &QScreen::logicalDotsPerInchChanged, [this]() {
        onCBoxCamerasCurrentIndexChanged(ui->cbox_camera->currentIndex());
    });

    connect(&GSignalSlot, &GlobalSignalSlot::IpcParamsErrorSig, this, &CMainWindow::onIpcParamsError);
    connect(&GSignalSlot, &GlobalSignalSlot::crashed, this, &CMainWindow::onCrashed);
    connect(&GSignalSlot, &GlobalSignalSlot::concatStarted, [this]() { ++m_concatProcessNum; });
    connect(&GSignalSlot, &GlobalSignalSlot::concatDone, [this]() { --m_concatProcessNum; });
    
    initUi();

    //����ʱ�䶨ʱ��
    m_updateTimeTimer = new QTimer(this);
    connect(m_updateTimeTimer, &QTimer::timeout, this, &CMainWindow::onTimerUpdateServerTime);
    if (!CONFIG.enableServer())
    {
        m_updateTimeTimer->start(1000);
    }

    QTimer::singleShot(300, [this]() { 
        if (!CONFIG.recordVideoEnabled())
        {
            CMessageBox::info(qstr("¼����ƵĿ¼���ڴ���ռ�ÿռ䳬����80%\n������¼����Ƶ��"));
        }
    });

}

void CMainWindow::aboutToQuit()
{
}

CMainWindow::~CMainWindow()
{
    delete m_pageNavigator;
    m_pageNavigator = nullptr;
    delete ui;
    qDebug() << "~CMainWindow()";
}

void CMainWindow::initQAction()
{
    m_recordDirAct = new QAction(this);
    m_recordDirAct->setText(qstr("¼����Ƶ"));
    connect(m_recordDirAct, &QAction::triggered, []() {
        QString path = CONFIG.recordPath();
        QDir dir(path);
        if (!dir.exists())
        {
            if (!dir.mkpath(path))
            {
                QString msg = qstr("����Ŀ¼%1ʧ��").arg(path);
                qInfo() << msg;
                CMessageBox::info(msg);
                return;
            }
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
    m_logAct = new QAction(this);
    m_logAct->setText(qstr("��־"));
    connect(m_logAct, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(util::logDir()));
    });
}

void CMainWindow::initQComboBox()
{
    QListWidget *listWidget = new QListWidget(this);
    QStringList sl = CONFIG.screenList();
    for (int i = 0; i < sl.count(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(sl.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        listWidget->addItem(item);
    }

    ui->cbox_camera->setModel(listWidget->model());
    ui->cbox_camera->setView(listWidget);

    m_cameraEdit = new QLineEdit(this);
    m_cameraEdit->setReadOnly(true);
    m_cameraEdit->setAlignment(Qt::AlignCenter);
    m_cameraEdit->setFocusPolicy(Qt::NoFocus);
    ui->cbox_camera->setLineEdit(m_cameraEdit);
    QLineEdit::connect(m_cameraEdit, &QLineEdit::selectionChanged, [=] {
        m_cameraEdit->deselect();
    });
    ui->cbox_camera->lineEdit()->installEventFilter(this);
    this->installEventFilter(this);

    connect(ui->cbox_camera, QOverload<int>::of(&QComboBox::activated),
        [=](int index) {
        m_showPopup = true;
    });

    connect(ui->cbox_camera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CMainWindow::onCBoxCamerasCurrentIndexChanged);
}

void CMainWindow::initUi()
{
    if (CONFIG.enableServer())
    {
        newLoadingShade();
    }

    ui->setupUi(this);

    m_pageNavigator = new SimplePageNavigator();
    connect(m_pageNavigator, &SimplePageNavigator::currentPageChanged, ui->page_1, &CCenter::currentPageChanged);
    m_pageNavigator->setCurrentPage(1);
    ui->selectPageLayout->addWidget(m_pageNavigator);
    ui->pageFrame->installEventFilter(this);

    initQComboBox();

    util::resetMaxWidth(ui->label_info, 600);
    util::setTextWithEllipsis(ui->label_info, ui->label_info->text());

    initQAction();

    if (CONFIG.enableServer())
    {
        showLoadingShade();
    }


	connect(ui->page_1, &CCenter::focus, this, &CMainWindow::onFocus);
    connect(ui->page_2, &CBigScreen::back, this, &CMainWindow::onBack);
    connect(ui->btn_setting, &QPushButton::clicked, this, &CMainWindow::onBtnSetting);


    QString logoUrl = CONFIG.themeValue(KEY_LOGO);
    if (logoUrl.isEmpty())
    {
        ui->logoLabel->setPixmap(CONFIG.loginLogo());
    }
    else
    {
        ui->logoLabel->setPixmap(CONFIG.logoPixmap());
    }

    //QString bgColor = CONFIG.themeBgColor();
    //ui->btn_cmd->setStyleSheet(bgColor);

     QLabel* iconLabel = new QLabel;
    QLabel* textLabel = new QLabel;
     iconLabel->setFixedSize(16, 16);
     iconLabel->setPixmap(QPixmap(":/down_arrow.png").scaled(10, 10, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->setFixedHeight(20);
    textLabel->setStyleSheet("font-size:13px;");
    QHBoxLayout* myLayout = new QHBoxLayout();
    myLayout->setContentsMargins(0, 0, 0, 0);
    myLayout->setSpacing(10);
    myLayout->addStretch();
    myLayout->addWidget(textLabel);
    myLayout->addWidget(iconLabel);


    ui->page_1->openAll();

#if 1
    ui->logoLabel->hide();
    ui->label_info->hide();
#endif
}

void CMainWindow::updateUi()
{
    ui->page_1->loadData();

    util::setTextWithEllipsis(ui->label_info, "xxx");
    
    refreshBtnCmd();
}

void CMainWindow::newLoadingShade()
{
    m_loadingLabel = new LoadingLabel(this);
    m_movie = new QMovie(":/loading.gif", QByteArray(), this);
    m_loadingLabel->setMovie(m_movie);
}

void CMainWindow::showLoadingShade()
{
    if (m_loadingLabel && m_movie)
    {
        m_loadingLabel->show();
        m_movie->start();
        m_loadingLabel->raise();
        m_useShade = true;
    }
}

void CMainWindow::hideLoadingShade()
{
    if (m_loadingLabel && m_movie)
    {
        m_movie->stop();
        m_loadingLabel->hide();
        m_useShade = false;
    }
}

void CMainWindow::showEvent(QShowEvent *event)
{
    int index = ui->cbox_camera->findText(CONFIG.defaultScreenNumText());
    switch (index) {
    case 0:
        m_pageNavigator->setMaxPage(1);
        ui->pageFrame->hide();
        ui->page_1->switch24Screen();
        break;
    case 1:
        m_pageNavigator->setMaxPage(2);
        ui->pageFrame->show();
        ui->page_1->switch12Screen();
        break;
    case 2:
        m_pageNavigator->setMaxPage(3);
        ui->pageFrame->show();
        ui->page_1->switch8Screen();
        break;
    case 3:
        m_pageNavigator->setMaxPage(1);
        ui->pageFrame->hide();
        ui->page_1->switch28Screen();
        break;
    default: break;
    }

    // ���ⷢ��currentIndexChanged�ź�
    ui->cbox_camera->blockSignals(true);
    ui->cbox_camera->setCurrentIndex(index);
    ui->cbox_camera->blockSignals(false);

    QMainWindow::showEvent(event);
}

void CMainWindow::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    if(m_loadingLabel && m_useShade)
    {
        m_loadingLabel->move((width() - m_loadingLabel->width()) / 2, (height() - m_loadingLabel->height()) / 2);
    }
}

void CMainWindow::paintEvent(QPaintEvent *ev)
{
    if (m_useShade)
    {
        //�����滭����
        QPainter painter(this);
        //Ϊ�������һ����͸���ľ�������
        painter.fillRect(this->rect(), QColor(0, 0, 0, 100));

        update();
    }
    else
    {
        QWidget::paintEvent(ev);
    }
}

void CMainWindow::refreshBtnCmd()
{
    switch (m_recordStarted) {
    case 0:
        ui->btn_cmd->setText(qstr("��ʼ¼��"));
        break;
    case 1:
        ui->btn_cmd->setText(qstr("����¼��"));
        break;
    }
}

void CMainWindow::showInfo(bool ifshow)
{
    if(ifshow)
    {
    }
    else
    {
        QTimer::singleShot(1000, [this]() {
            update();
        });

        ui->cbox_camera->lineEdit()->removeEventFilter(this);
        this->removeEventFilter(this);
    }

    ui->btn_cmd->setVisible(ifshow);
    ui->label_date->setVisible(ifshow);
    ui->label_time->setVisible(ifshow);
    if (m_cameraEdit)
    {
        m_cameraEdit->setEnabled(ifshow);
    }

}

void CMainWindow::startRecord()
{
    if (!m_recordStarted)
    {
        m_recordStarted = true;
        ui->page_1->allPeopleStart();
    }
}

void CMainWindow::endRecord()
{
    if(m_recordStarted)
    {
        m_recordStarted = false;
        ui->page_1->allPeopleStop();
    }
}

void CMainWindow::updateTimeLabel()
{
    QDateTime cdt;
    if (CONFIG.enableServer())
    {
    }
    else
    {
        cdt = QDateTime::currentDateTime();
    }

    ui->label_time->setText(cdt.toString("hh:mm:ss"));
    ui->label_date->setText(cdt.toString("yyyy-MM-dd"));
}

void CMainWindow::updateTime()
{
    updateTimeLabel();
}

void CMainWindow::setUiBlank()
{
    ui->page_1->loadData();
}

void CMainWindow::onBtnClose()
{
    if (m_concatProcessNum > 0) {
        CMessageBox::info(qstr("��������¼����Ƶ��\n���Ժ��ٹر�"));
        return;
    }

    if (CMessageBox::confirm(qstr("�Ƿ�ȷ���˳���")))
    {
        close();
    }
}

void CMainWindow::onBtnCmd()
{
    //int status = 0;
    QString text;
    switch (m_recordStarted) {
    case 0: text = qstr("�Ƿ�ȷ�Ͽ�ʼ¼�ƣ�"); break;
    case 1:  text = qstr("�Ƿ�ȷ�Ͻ���¼�ƣ�"); break;
    }

    if (!CMessageBox::confirm(text)) return;

    ui->btn_cmd->setEnabled(false);
    QTimer::singleShot(CONFIG.startBtnDisabledTime(), [this]() { ui->btn_cmd->setEnabled(true); });

    switch (m_recordStarted) {
    case 0:
        startRecord();
        break;
    case 1:
        endRecord();
        break;
    }
    refreshBtnCmd();
}

void CMainWindow::onBtnSetting()
{
    static const QString s_color = CONFIG.themeColorValue();
    static const QString s_qss = QString("QMenu { \
        background-color:rgb(89,87,87); \
        } \
        QMenu::item{ \
        font-size: 10pt; \
        color: rgb(225,225,225);\
        background-color:rgb(89,87,87); \
        padding:6px 20px 6px 20px;\
        }\
        QMenu::item:selected{ \
        background-color:%1;\
        }\
        QMenu::item:pressed{\
        border: 1px solid rgb(60,60,61); \
        background-color: rgb(220,80,6); \
        }\
        QMenu::separator{\
        height: 1px;\
        background: black;\
        }")
        .arg(s_color);

    QMenu settingMenu;
    settingMenu.setStyleSheet(s_qss);

    settingMenu.addAction(m_logAct);
    settingMenu.addSeparator();
    settingMenu.addAction(m_recordDirAct);

    settingMenu.exec(QCursor::pos());
}

void CMainWindow::onCBoxCamerasCurrentIndexChanged(int idx)
{
    switch (idx) {
    case 0:
        switch24Screen();
        break;
    case 1:
        switch12Screen();
        break;
    case 2:
        switch8Screen();
        break;
    case 3:
        switch28Screen();
        break;
    default: break;
    }
}

void CMainWindow::openAllVideos()
{
    if (CMessageBox::confirm(qstr("�Ƿ����������ͷ��")))
    {
        qInfo() << qstr("�ֶ�����������ͷ");
        ui->page_1->openAll();
    }
}

void CMainWindow::closeAllVideos()
{
    if (CMessageBox::confirm(qstr("�Ƿ�ر���������ͷ��")))
    {
        qInfo() << qstr("�ֶ��ر���������ͷ");
        ui->page_1->closeAll();
    }
}

void CMainWindow::switch24Screen()
{
    QTimer::singleShot(1, [this]() {
        ui->pageFrame->hide();

        QTimer::singleShot(100, [this]() {
            ui->page_1->switch24Screen();
        });
    });
}

void CMainWindow::switch12Screen()
{

    QTimer::singleShot(1, [this]() {
        m_pageNavigator->setMaxPage(2);
        ui->pageFrame->show();

        QTimer::singleShot(100, [this]() {
        ui->page_1->switch12Screen();
        });
    });
}

void CMainWindow::switch8Screen()
{
    QTimer::singleShot(1, [this]() {
        m_pageNavigator->setMaxPage(3);
        ui->pageFrame->show();

        QTimer::singleShot(100, [this]() {
            ui->page_1->switch8Screen();
        });
    });

}

void CMainWindow::switch28Screen()
{
    QTimer::singleShot(1, [this]() {
        ui->pageFrame->hide();

        QTimer::singleShot(100, [this]() {
            ui->page_1->switch28Screen();
        });
    });

}

void CMainWindow::onTimerUpdateServerTime()
{
    updateTimeLabel();
}

void CMainWindow::onFocus(QVariantMap data)
{
    ui->page_2->init(data);
    ui->page_2->startPlay();
    ui->stackedWidget->setCurrentIndex(1);
    ui->stackedWidget->setWindowFlags(Qt::Window);
    ui->stackedWidget->showFullScreen();
}

void CMainWindow::onBack()
{
    qInfo() << qstr("�˳�ȫ��");

    // ���²�����Ƶ
    ui->page_1->pause(false);

    // ����ҳ���ٻ�ԭ����ֹ�ٴδ�����ǰҳ���showEvent
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->setWindowFlags(Qt::Widget);
    ui->stackedWidget->showNormal();
}

bool CMainWindow::eventFilter(QObject* target, QEvent* event) {
    QEvent::Type type = event->type();
    if (QEvent::MouseButtonRelease == type) {
        if (QString(target->metaObject()->className()) == "QLineEdit") {
            if (m_showPopup) {
                ui->cbox_camera->showPopup();
                m_showPopup = false;
            } else {
                ui->cbox_camera->hidePopup();
                m_showPopup = true;
            }
        } else {
            m_showPopup = true;
        }
        return true;
    } else if (QEvent::HideToParent == type && target == ui->pageFrame) {
        ui->page_1->resize(ui->page_1->width(), ui->page_1->height() + ui->pageFrame->height());
    }
    return QWidget::eventFilter(target, event);
}

void CMainWindow::onIpcParamsError(const QString &msg)
{
    CMessageBox::info(msg);
}

void CMainWindow::onCrashed()
{
    qCritical() << qstr("����������");
    // ����ʱ����¼��
    ui->page_1->allPeopleStop();
}

void CMainWindow::currentPageChanged(int page)
{
    showLoadingShade();
    QTimer::singleShot(300, [this]() {
        hideLoadingShade();
    });
}
