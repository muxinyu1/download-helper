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
  /// 创建下载任务，并加入下载队列
  /// </summary>
  /// <param name="url">要下载文件的url</param>
  /// <param name="output">输出的文件夹</param>
  /// <param name="concurrency">线程数</param>
  /// <param name="speedLimit">是否限速</param>
  /// <param name="speed">下载速度(KB/S)</param>
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
  /// 合并一个task的各个线程的文件
  /// </summary>
  /// <param name="taskId">taskId</param>
  void combineFiles(int taskId);
  /// <summary>
  /// 改变详情面板显示的下载任务
  /// </summary>
  /// <param name="item"></param>
  void changeDetail(QListWidgetItem *item);
  void markTaskAsOk(int taskId);
  void showAddTaskWindow();
  void addTask(QString urls, int concurrecny, bool speedLimit, int speed);
  void removeTask(int taskId);
};
