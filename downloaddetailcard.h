#pragma once

#include "ui_downloaddetailcard.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class DownloadDetailCardClass;
};
QT_END_NAMESPACE

class DownloadDetailCard : public QWidget {
  Q_OBJECT

public:
  DownloadDetailCard(QString threadIndex, QWidget *parent = nullptr);
  ~DownloadDetailCard();

  // getter
  QProgressBar *getProgressBar();

private:
  Ui::DownloadDetailCardClass *ui;
};
