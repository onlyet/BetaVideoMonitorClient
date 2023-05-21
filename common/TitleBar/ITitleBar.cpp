#include "ITitleBar.h"

#include <QMouseEvent>

ITitleBar::ITitleBar(QWidget *parent):
    QFrame(parent)
{
    setWidget(parent);
    setContentsMargins(0,0,0,0);
}

ITitleBar::~ITitleBar()
{

}

void ITitleBar::setWidget(QWidget *wid)
{
    Q_ASSERT(wid != Q_NULLPTR);
    m_pMoveWidget = wid;
}

void ITitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isMousePressed && (event->buttons() & Qt::LeftButton))
    {
        m_pMoveWidget->move(event->globalPos() - m_mousePoint);
        event->accept();
    }
}

void ITitleBar::mousePressEvent(QMouseEvent *event)
{
    if (m_pMoveWidget == Q_NULLPTR) return;

    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = true;
        m_mousePoint = event->globalPos() - m_pMoveWidget->pos();
        event->accept();
    }
}

void ITitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = false;
    }
}
