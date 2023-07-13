#pragma once

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <algorithm>
#include "downloadcard.h"
#include "downloadthread.h"
#include "downloaddetailcard.h"

class TaskState : public QObject {
  Q_OBJECT

public:
  TaskState(DownloadCard* card, QHash<int, bool> threadState, QObject *parent = nullptr);
  ~TaskState();

  // getter
  DownloadCard *getCard();
  QHash<int, bool> &getThreadState();
  QList<DownloadThread*> &getThreads();
  QList<DownloadDetailCard *> &getDetails();

  // setter
  void setBytesTotal(qint64 bytesTotal);
  void setBytesEachThread(int concurrency);

  void updateProgressBar(int threadIndex, qint64 bytesRecieved);
  void updateDetailCard(int threadIndex, qint64 bytesRecieved,
                        qint64 bytesTotal);
  void setOkVisible();
  void combine();

private:
  DownloadCard *card;
  QHash<int, bool> threadState;
  qint64 bytesTotal;
  QMutex mutex;
  QList<DownloadThread*> threads;
  QHash<int, qint64> bytesEachThread;
  QList<DownloadDetailCard*> details;

  qint64 getBytesDownloaded();


};
