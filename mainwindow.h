#pragma once

#include "downloadthread.h"
#include "ui_mainwindow.h"
#include <QHash>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindowClass;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  MainWindow(QString url, QString output, int concurrency,
             QWidget *parrent = nullptr);
  ~MainWindow();

  void createDownloadTask(QString url, QString output, int concurrency);

private:
  Ui::MainWindowClass *ui;
  QNetworkAccessManager *manager;
  QHash<int, QHash<int, bool>> tasks;
  int currentTaskId;

private:
  void Download(int taskId, QString url, QString output, int concurrency);

signals:
  void taskFinished(int taskId);
private slots:
  void updateTaskState(int taskId, int threadIndex);
};
