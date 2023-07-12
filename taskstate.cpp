#include "taskstate.h"

TaskState::TaskState(DownloadCard *card, QHash<int, bool> threadState,
                     QObject *parent)
    : QObject(parent), card(card), threadState(threadState), bytesTotal(0),
      mutex(), threads(), bytesEachThread() {}

TaskState::~TaskState() {
  for (auto thread : threads)
    delete thread;
}

DownloadCard *TaskState::getCard() { return card; }

QHash<int, bool> &TaskState::getThreadState() { return threadState; }

QList<DownloadThread *> &TaskState::getThreads() { return threads; }

void TaskState::setBytesTotal(qint64 bytesTotal) {
  this->bytesTotal = bytesTotal;
}

void TaskState::setBytesEachThread(int concurrency) {
  auto blockSize = bytesTotal / concurrency;
  for (int i = 0; i < concurrency; ++i) {
    bytesEachThread.insert(i, i == concurrency - 1
                                  ? (blockSize + (bytesTotal % concurrency))
                                  : blockSize);
  }
}

void TaskState::updateProgressBar(int threadIndex, qint64 bytesRecieved) {
  mutex.lock();
  bytesEachThread[threadIndex] = bytesRecieved;
  auto bytesDownloaded = getBytesDownloaded();
  qDebug() << QString{"task updated bar, bytesDownloaded = %1, total = %2"}
                  .arg(bytesDownloaded)
                  .arg(bytesTotal);
  auto percent = static_cast<int>((bytesDownloaded * 100) / bytesTotal);
  card->updateProgressBar(percent);
  mutex.unlock();
}

void TaskState::combine() { // TODO: combine нд╪Ч
}

qint64 TaskState::getBytesDownloaded() {
  auto sum = std::accumulate(bytesEachThread.begin(), bytesEachThread.end(),
                             (qint64)0);
  return sum;
}
