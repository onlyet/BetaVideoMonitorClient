#include "LoadingLabel.h"

LoadingLabel::LoadingLabel(QWidget *parent)
	: QLabel(parent)
	, m_label(new QLabel(this))
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setAlignment(Qt::AlignCenter);
	setFixedSize(300, 300);
}

LoadingLabel::~LoadingLabel()
{
}
