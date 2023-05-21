#pragma once
#include <QWidget>

namespace Ui {
    class LoadingWidget;
}
class QMovie;

class LoadingWidget : public QWidget
{
	Q_OBJECT

public:
    LoadingWidget(QWidget *parent = Q_NULLPTR);
    ~LoadingWidget();

    void showLoadingShade();
    void hideLoadingShade();

protected:
//	void resizeEvent(QEvent *ev) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *ev) override;


private:
    Ui::LoadingWidget   *ui;
    QMovie              *m_movie = nullptr;
};
