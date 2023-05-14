#include "IMessageBox.h"

#include <QMouseEvent>

IMessageBox::IMessageBox(QWidget* parent) :
    QDialog(parent)
{
    //setWindowFlag(Qt::FramelessWindowHint, true);
    //setWindowFlags(Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground);
}

IMessageBox::~IMessageBox()
{

}

void IMessageBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = true;
        m_mousePoint = event->globalPos() - pos();
        event->accept();
    }
}

void IMessageBox::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isMousePressed && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_mousePoint);
        event->accept();
    }
}

void IMessageBox::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = false;
    }
}
