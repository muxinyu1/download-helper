#include "downloadcard.h"

DownloadCard::DownloadCard(QString filename, QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadCardClass()) {
  ui->setupUi(this);
  ui->label->setText(filename);
  ui->ok->hide();
  connect(ui->close, &QPushButton::clicked, this, [this]() { emit remove(); });
}

DownloadCard::~DownloadCard() { delete ui; }

void DownloadCard::updateProgressBar(int percent) {
  ui->progressBar->setValue(percent);
}

void DownloadCard::setOkVisiable() { ui->ok->show(); }
