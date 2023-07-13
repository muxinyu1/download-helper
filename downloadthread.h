#pragma once

#include <QDir>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

class DownloadThread : public QThread {
  Q_OBJECT

public:
  DownloadThread(int taskId, int threadIndex, QString url, qlonglong begin,
                 qlonglong end, QObject *parrent = nullptr);
  ~DownloadThread();

  void stop();

protected:
  void run() override;

private:
  int taskId;
  int threadIndex;
  QString url;
  qlonglong begin;
  qlonglong end;
  bool stopped;

  void downloadPart();
  void saveToTempDir(const QByteArray &bytes);
  void deleteTemp();

signals:
  void downloadFinished(int taskId, int threadIndex);
  void downloadSize(int taskId, int threadIndex, qint64 downloadedSize);
  void downloadProgress(int taskId, int threadIndex, qint64 bytesReceived,
                        qint64 bytesTotal);
  void stopSelf();
};
