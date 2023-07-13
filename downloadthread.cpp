#include "downloadthread.h"

DownloadThread::DownloadThread(int taskId, int threadIndex, QString url,
                               qlonglong begin, qlonglong end, QObject *parrent)
    : QThread(parrent), taskId(taskId), threadIndex(threadIndex), url(url),
      begin(begin), end(end), loop(new QEventLoop(this)) {}

DownloadThread::~DownloadThread() {
  qDebug() << QString{"thread{%1} exits"}.arg(threadIndex);
}
void DownloadThread::stop() {
  deleteTemp();
  loop->quit();
}
void DownloadThread::run() { downloadPart(); }

void DownloadThread::downloadPart() {
  qDebug() << QString{"thread {%1} starts downloading part{%2-%3}"}
                  .arg(threadIndex)
                  .arg(begin)
                  .arg(end);

  QNetworkRequest request{QUrl{url}};
  auto bytesRange = QString{"bytes=%1-%2"}.arg(begin).arg(end).toStdString();
  request.setRawHeader("Range", bytesRange.c_str());

  qDebug() << QString{"sent Range: {%1}-{%2}"}.arg(begin).arg(end);

  auto manager =
      QSharedPointer<QNetworkAccessManager>{new QNetworkAccessManager()};
  auto reply = manager->get(request);
  connect(reply, &QNetworkReply::downloadProgress,
          [this](qint64 bytesReceived, qint64 bytesTotal) {
            qDebug() << QString{"Thread{%1} received{%2}, total{%3}"}
                            .arg(threadIndex)
                            .arg(bytesReceived)
                            .arg(bytesTotal);

            emit downloadSize(taskId, threadIndex, bytesReceived);
            emit downloadProgress(taskId, threadIndex, bytesReceived,
                                  bytesTotal);
          });
  connect(reply, &QNetworkReply::finished, [reply, this]() {
    if (reply->error() != QNetworkReply::NoError) {
      // TODO: 单个线程下载错误
    }

    qDebug() << QString{"Thread {%1} reply finished"}.arg(threadIndex);

    auto bytes = reply->readAll();
    saveToTempDir(bytes);

    qDebug() << QString{"Thread{%1} ok"}.arg(threadIndex);

    emit downloadFinished(taskId, threadIndex);
  });

  connect(reply, &QNetworkReply::finished, loop, [this, manager]() {
    manager->deleteLater();
    loop->quit();
  });
  loop->exec();
}

void DownloadThread::saveToTempDir(const QByteArray &bytes) {
  auto tempPath = QDir::tempPath();
  auto filename = QUrl(url).fileName() + QString{".part%1"}.arg(threadIndex);
  QFile file{tempPath + "/" + filename};
  if (!file.open(QIODevice::WriteOnly)) {
    // TODO: 打开文件失败
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
