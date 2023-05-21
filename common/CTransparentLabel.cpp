#include "CTransparentLabel.h"

#include <QPainter>
#include <QDebug>

CTransparentLabel::CTransparentLabel(const QString &text, QWidget *parent)
    : QLabel(parent)
    , m_text(text)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAlignment(Qt::AlignCenter);
}

CTransparentLabel::~CTransparentLabel()
{
}

void CTransparentLabel::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void CTransparentLabel::setPaintText(const QString &text)
{
    m_text = text;
    update();
}

void CTransparentLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 128));
    QPen pen;
    pen.setColor(m_color);
    painter.setPen(pen);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}
