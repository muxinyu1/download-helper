#pragma once

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <algorithm>
#include <QFileDialog>
#include "downloadcard.h"
#include "downloadthread.h"
#include "downloaddetailcard.h"

class TaskState : public QObject {
  Q_OBJECT

public:
  TaskState(int taskId, DownloadCard* card, QHash<int, bool> threadState, QString filename, QObject *parent = nullptr);
  ~TaskState();

  void addThread(DownloadThread *thread);
  int getThreadNum();

  // getter
  DownloadCard *getCard();
  QHash<int, bool> &getThreadState();
  QList<DownloadDetailCard *> &getDetails();
  QString getFilename();

  // setter
  void setBytesTotal(qint64 bytesTotal);
  void setBytesEachThread(int concurrency);

  void updateProgressBar(int threadIndex, qint64 bytesRecieved);
  void updateDetailCard(int threadIndex, qint64 bytesRecieved,
                        qint64 bytesTotal);
  void setOkVisible();

signals:
  void remove(int taskId);

private:
  int taskId;
  DownloadCard *card;
  QHash<int, bool> threadState;
  qint64 bytesTotal;
  QMutex mutex;
  QList<DownloadThread*> threads;
  QHash<int, qint64> bytesEachThread;
  QList<DownloadDetailCard*> details;
  QString filename;

  qint64 getBytesDownloaded();
};
