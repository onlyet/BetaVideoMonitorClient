#ifndef TOAST_H
#define TOAST_H

#include <QWidget>

namespace Ui
{
    class Toast;
};
class QLabel;
class QPropertyAnimation;

// 定时自动消失的提示框
class Toast : public QWidget
{
	Q_OBJECT

public:
	Toast(const QString &msg, QWidget *parent, int duration);
	static void showMessage(const QString &msg, QWidget *parent = nullptr, int msecs = 2000);

protected:
#if 0
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
#endif

private:
    Ui::Toast *ui;
	QPropertyAnimation *m_animation;
};

#endif // !TOAST_H