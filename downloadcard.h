#pragma once

#include "ui_downloadcard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DownloadCardClass; };
QT_END_NAMESPACE

class DownloadCard : public QWidget
{
	Q_OBJECT

public:
	DownloadCard(QWidget *parent = nullptr);
	~DownloadCard();

	void updateProgressBar(int percent);

private:
	Ui::DownloadCardClass *ui;
};
