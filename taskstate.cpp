#include "taskstate.h"

TaskState::TaskState(DownloadCard *card, QHash<int, bool> threadState,
                     QObject *parent)
    : QObject(parent), card(card), threadState(threadState), bytesTotal(0),
      mutex(), threads(), bytesEachThread(), details() {
  for (int i = 0, len = threadState.size(); i < len; ++i) {
    auto detail = new DownloadDetailCard(QString{"Thread %1"}.arg(i));
    details.push_back(detail);
  }
}

TaskState::~TaskState() {
  for (auto thread : threads)
    delete thread;
  for (auto detail : details) {
    delete detail;
  }
}

DownloadCard *TaskState::getCard() { return card; }

QHash<int, bool> &TaskState::getThreadState() { return threadState; }

QList<DownloadThread *> &TaskState::getThreads() { return threads; }

QList<DownloadDetailCard *> &TaskState::getDetails() { return details; }

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

void TaskState::updateDetailCard(int threadIndex, qint64 bytesRecieved,
                                 qint64 bytesTotal) {
  auto progressBar = details[threadIndex]->getProgressBar();

  if (bytesRecieved >= bytesTotal) {
    progressBar->setValue(100);
  }

  auto percent = static_cast<int>((bytesRecieved * 100) / bytesTotal);
  progressBar->setValue(percent);
}

void TaskState::setOkVisible() { card->setOkVisiable(); }

void TaskState::combine() { // TODO: combine нд╪Ч
}

qint64 TaskState::getBytesDownloaded() {
  auto sum = std::accumulate(bytesEachThread.begin(), bytesEachThread.end(),
                             (qint64)0);
  return sum;
}
