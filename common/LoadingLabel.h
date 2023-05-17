#pragma once

#include <QLabel>

class LoadingLabel : public QLabel
{
	Q_OBJECT

public:
    LoadingLabel(QWidget *parent = Q_NULLPTR);
    ~LoadingLabel();

private:
	QLabel *m_label;
};
