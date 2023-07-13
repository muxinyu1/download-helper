#include "downloaddetailcard.h"

DownloadDetailCard::DownloadDetailCard(QString threadIndex, QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadDetailCardClass()) {

  ui->setupUi(this);
  ui->threadIndex->setText(threadIndex);
}

DownloadDetailCard::~DownloadDetailCard() { delete ui; }

QProgressBar *DownloadDetailCard::getProgressBar() { return ui->progressBar; }
