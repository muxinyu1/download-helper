#include "downloadthread.h"

DownloadThread::DownloadThread(int taskId, int threadIndex, QString url, qlonglong begin,
                               qlonglong end,
                               QProgressBar *progressBar, QObject *parrent)
    : QThread(parrent), taskId(taskId), threadIndex(threadIndex), url(url), begin(begin), end(end),
      progressBar(progressBar), manager(new QNetworkAccessManager()) {}

DownloadThread::~DownloadThread() { delete manager; }

void DownloadThread::run() { DownloadPart(); }

void DownloadThread::DownloadPart() {
  QNetworkRequest request{QUrl{url}};
  auto bytesRange = QString{"bytes=%1-%2"}.arg(begin).arg(end).toUtf8();
  request.setRawHeader("Range", bytesRange);
  auto reply = manager->get(request);
  connect(reply, &QNetworkReply::downloadProgress, this,
          [this](qint64 bytesReceived, qint64 bytesTotal) {
            if (bytesReceived >= bytesTotal) {
              progressBar->setValue(100);
            }
            if (bytesTotal > 0) {
              auto persent =
                  static_cast<int>((bytesReceived * 100) / bytesTotal);
              progressBar->setValue(persent);
            }
          });
  connect(reply, &QNetworkReply::isFinished, this, [reply, this]() {
    if (reply->error() != QNetworkReply::NoError) {
      // TODO: 单个线程下载错误
    }
    auto bytes = reply->readAll();
    SaveToTempDir(bytes);
    emit downloadFinished(taskId, threadIndex);
  });
}

void DownloadThread::SaveToTempDir(const QByteArray &bytes) {
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
