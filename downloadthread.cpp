#include "downloadthread.h"

DownloadThread::DownloadThread(int taskId, int threadIndex, QString url,
                               qlonglong begin, qlonglong end,
                               QProgressBar *progressBar, QObject *parrent)
    : QThread(parrent), taskId(taskId), threadIndex(threadIndex), url(url),
      begin(begin), end(end), progressBar(progressBar) {}

DownloadThread::~DownloadThread() {
  qDebug() << QString{"thread{%1} exits"}.arg(threadIndex);
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
            if (progressBar == nullptr) {
              return;
            }
            if (bytesReceived >= bytesTotal) {
              progressBar->setValue(100);
            }
            if (bytesTotal > 0) {
              auto persent =
                  static_cast<int>((bytesReceived * 100) / bytesTotal);
              progressBar->setValue(persent);
            }
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

  QEventLoop loop{};

  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
}

void DownloadThread::saveToTempDir(const QByteArray &bytes) {
  auto tempPath = QDir::tempPath();
  auto filename = QFileInfo{QUrl(url).path()}.fileName() +
                  QString{".part%1"}.arg(threadIndex);
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
