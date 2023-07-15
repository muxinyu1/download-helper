#include "downloadthread.h"

DownloadThread::DownloadThread(int taskId, int threadIndex, QString url,
                               qlonglong begin, qlonglong end, QObject *parrent)
    : QThread(parrent), taskId(taskId), threadIndex(threadIndex), url(url),
      begin(begin), end(end), stopped(false), pos(begin), newPos(begin) {}

DownloadThread::~DownloadThread() {
  qDebug() << QString{"thread{%1} exits"}.arg(threadIndex);
  quit();
  wait();
}
void DownloadThread::stop() {
  deleteTemp();
  // TODO: 释放reply
  requestInterruption();
  emit stopSelf();
}
void DownloadThread::pause() {
  pos = newPos;
}
void DownloadThread::resume() {}
void DownloadThread::run() { downloadPart(begin, end); }

void DownloadThread::downloadPart(qint64 begin, qint64 end) {
  if (begin > end) {
    return;
  }
  qDebug() << QString{"thread {%1} starts downloading part{%2-%3}"}
                  .arg(threadIndex)
                  .arg(begin)
                  .arg(end);

  QNetworkRequest request{QUrl{url}};
  auto bytesRange = QString{"bytes=%1-%2"}.arg(begin).arg(end).toStdString();
  if (QUrl(url).scheme() == QLatin1String("ftp")) {
    request.setAttribute(QNetworkRequest::CustomVerbAttribute, "REST 0");
  }
  request.setRawHeader("Range", bytesRange.c_str());

  qDebug() << QString{"sent Range: {%1}-{%2}"}.arg(begin).arg(end);

  auto manager =
      QSharedPointer<QNetworkAccessManager>{new QNetworkAccessManager()};
  auto reply = manager->get(request);
  connect(reply, &QNetworkReply::downloadProgress, this,
          &DownloadThread::downloadProgressOfReply);
  connect(reply, &QNetworkReply::finished, [this, reply]() {
    if (isInterruptionRequested())
      return;
    if (reply->error() != QNetworkReply::NoError) {
      // TODO: 单个线程下载错误
    }

    qDebug() << QString{"Thread {%1} reply finished"}.arg(threadIndex);

    auto bytes = reply->readAll();
    qDebug() << "read ok from " << reply;
    saveToTempDir(bytes);

    qDebug() << QString{"Thread{%1} ok"}.arg(threadIndex);

    emit downloadFinished(taskId, threadIndex);
      });

  QEventLoop loop{};
  connect(reply, &QNetworkReply::finished, &loop, [&loop, manager]() {
    manager->deleteLater();
    loop.quit();
  });
  connect(this, &DownloadThread::stopSelf, &loop, [&loop, manager]() {
    // reply->abort();
    manager->deleteLater();
    loop.quit();
  });
  loop.exec();
}

void DownloadThread::saveToTempDir(const QByteArray &bytes) {
  auto tempPath = QDir::tempPath();
  auto filename = QUrl(url).fileName() + QString{".part%1"}.arg(threadIndex);
  QFile file{tempPath + "/" + filename};

  // 如果之前没暂停过
  if (pos == begin) {
    auto success = file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!success) {
      qDebug() << "open " << tempPath + "/" + filename << " failed.";
    }
  } else {
    // 暂停过就续写
    auto success = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (!success) {
      qDebug() << "open " << tempPath + "/" + filename << " failed.";
    }
  }
  auto bytesWritten = file.write(bytes);
  if (bytesWritten == -1) {
    // TODO: 写入失败
  }
  file.close();
}

void DownloadThread::deleteTemp() {
  auto tempFilename = QDir::tempPath() + "/" + QUrl(url).fileName() +
                      QString{".part%1"}.arg(threadIndex);
  QFile file{tempFilename};
  if (file.exists()) {
    file.remove();
  }
  file.close();
}

inline qint64 DownloadThread::getBytesDownloaded() { return newPos - begin; }

inline qint64 DownloadThread::getBytesTotal() { return end - begin + 1; }

void DownloadThread::handleReplyFinished() {
  //if (isInterruptionRequested())
  //  return;
  //if (reply->error() != QNetworkReply::NoError) {
  //  // TODO: 单个线程下载错误
  //}

  //qDebug() << QString{"Thread {%1} reply finished"}.arg(threadIndex);

  //auto bytes = reply->readAll();
  //qDebug() << "read ok from " << reply;
  //saveToTempDir(bytes);

  //qDebug() << QString{"Thread{%1} ok"}.arg(threadIndex);

  //emit downloadFinished(taskId, threadIndex);
}

void DownloadThread::downloadProgressOfReply(qint64 bytesReceived,
                                             qint64 bytesTotal) {
  newPos = pos + bytesReceived;

  qDebug() << QString{"Thread{%1} received{%2}, total{%3}"}
                  .arg(threadIndex)
                  .arg(bytesReceived)
                  .arg(bytesTotal);

  emit downloadSize(taskId, threadIndex, getBytesDownloaded());
  emit downloadProgress(taskId, threadIndex, getBytesDownloaded(),
                        getBytesTotal());
}
