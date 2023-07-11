#include "taskstate.h"

TaskState::TaskState(DownloadCard *card, QHash<int, bool> threadState,
                     QObject *parent)
    : QObject(parent), card(card), threadState(threadState), bytesTotal(0),
      bytesDownloaded(0), mutex(), threads() {}

TaskState::~TaskState() { for (auto thread: threads) delete thread; }

DownloadCard *TaskState::getCard() { return card; }

QHash<int, bool> &TaskState::getThreadState() { return threadState; }

QList<DownloadThread*> &TaskState::getThreads() {
  return threads;
}

void TaskState::setBytesTotal(qint64 bytesTotal) {
  this->bytesTotal = bytesTotal;
}

void TaskState::updateProgressBar(qint64 bytesRecieved) {
  mutex.lock();
  bytesDownloaded += bytesRecieved;
  auto percent = static_cast<int>((bytesDownloaded * 100) / bytesTotal);
  card->updateProgressBar(percent);
  mutex.unlock();
}

void TaskState::combine() { // TODO: combine нд╪Ч
}
