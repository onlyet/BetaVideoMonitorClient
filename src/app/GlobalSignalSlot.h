#pragma once

#include <QObject>

class GlobalSignalSlot : public QObject
{
    Q_OBJECT

public:
    GlobalSignalSlot(QObject *parent = nullptr);
    ~GlobalSignalSlot();

    static GlobalSignalSlot& instance() {
        static GlobalSignalSlot s_instance;
        return s_instance;
    }

    void notifyCrash();
    bool isCrashed() { return m_isCrashed; }

public slots:
    void IpcParamsErrorSlot(const QString& msg);

signals:
    void IpcParamsErrorSig(const QString& msg);
    void crashed();
    void concatStarted();
    void concatDone();

private:
    bool m_isCrashed = false;
};

#define GSignalSlot GlobalSignalSlot::instance()