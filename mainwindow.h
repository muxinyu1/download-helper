#pragma once

#include "downloadthread.h"
#include "taskstate.h"
#include "ui_mainwindow.h"
#include "addtaskwindow.h"
#include <QHash>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QSharedPointer>
#include <QFileDialog>
#include <QException>
#include <QStandardPaths>
#include <QDesktopServices>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  /// <summary>
  /// �����������񣬲��������ض���
  /// </summary>
  /// <param name="url">Ҫ�����ļ���url</param>
  /// <param name="output">������ļ���</param>
  /// <param name="concurrency">�߳���</param>
  /// <param name="speedLimit">�Ƿ�����</param>
  /// <param name="speed">�����ٶ�(KB/S)</param>
  void createDownloadTask(QString url, QString output, int concurrency, bool speedLimit, int speed);

private:
  Ui::MainWindow *ui;
  QNetworkAccessManager *manager;
  QHash<int, QSharedPointer<TaskState>> tasks;
  int currentTaskId;
  QHash<int, QListWidgetItem *>  listItemsFromTaskId;
  QListWidgetItem *currentItem;
  QHash<QListWidgetItem *, QListWidget *> currentDetail;

private:
  void Download(int taskId, QString url, QString output, int concurrency, bool speedLimit, int speed);
  QString getFilenameFromUrl(QString url);
  QListWidget *createDownloadDetail(int taskId);
  QString getSavedDir(const QString& filename);

signals:
  void taskFinished(int taskId);
private slots:
  void updateTaskState(int taskId, int threadIndex);
  void updateTaskProgressBar(int taskId, int threadIndex, qint64 downloadedSize);
  void updateTaskThreadDetail(int taskId, int threadIndex, qint64 bytesReceived,
                              qint64 bytesTotal);
  /// <summary>
  /// �ϲ�һ��task�ĸ����̵߳��ļ�
  /// </summary>
  /// <param name="taskId">taskId</param>
  void combineFiles(int taskId);
  /// <summary>
  /// �ı����������ʾ����������
  /// </summary>
  /// <param name="item"></param>
  void changeDetail(QListWidgetItem *item);
  void markTaskAsOk(int taskId);
  void showAddTaskWindow();
  void addTask(QString urls, int concurrecny, bool speedLimit, int speed);
  void removeTask(int taskId);
};
