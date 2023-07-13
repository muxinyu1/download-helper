#pragma once

#include <QWidget>
#include "ui_downloaddetail.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DownloadDetailClass; };
QT_END_NAMESPACE

class DownloadDetail : public QWidget
{
	Q_OBJECT

public:
	DownloadDetail(QWidget *parent = nullptr);
	~DownloadDetail();

private:
	Ui::DownloadDetailClass *ui;
};
