#include "downloadcard.h"

DownloadCard::DownloadCard(QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadCardClass()) {
  ui->setupUi(this);
}

DownloadCard::~DownloadCard() { delete ui; }

void DownloadCard::updateProgressBar(int percent) {
  ui->progressBar->setValue(percent);
}
