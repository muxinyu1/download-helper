#include "downloaddetail.h"

DownloadDetail::DownloadDetail(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DownloadDetailClass())
{
	ui->setupUi(this);
}

DownloadDetail::~DownloadDetail()
{
	delete ui;
}
