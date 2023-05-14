#pragma once

#include <QLabel>
#include <QFont>
#include <QPen>

class CTransparentLabel : public QLabel
{
    Q_OBJECT

public:
    CTransparentLabel(const QString &text, QWidget *parent = Q_NULLPTR);
    ~CTransparentLabel();

    void setColor(const QColor &color);
    void setPaintText(const QString &text);

protected:
    void paintEvent(QPaintEvent* e);


private:
    QString     m_text; // »æÖÆÎÄ±¾
    QColor      m_color{ Qt::white };
};
