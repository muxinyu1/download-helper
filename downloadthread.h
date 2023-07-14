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
  void pause();
  void resume();

protected:
  void run() override;

private:
  int taskId;
  int threadIndex;
  QString url;
  qlonglong begin;
  qlonglong end;
  bool stopped;
  qint64 pos;
  qint64 newPos;
  QNetworkReply *reply;

  void downloadPart(qint64 begin, qint64 end);
  void saveToTempDir(const QByteArray &bytes);
  void deleteTemp();
  inline qint64 getBytesDownloaded();
  inline qint64 getBytesTotal();
private slots:
  void downloadProgressOfReply(qint64 bytesRecieved, qint64 bytesTotal);
  void handleReplyFinished();

signals:
  void downloadFinished(int taskId, int threadIndex);
  void downloadSize(int taskId, int threadIndex, qint64 downloadedSize);
  void downloadProgress(int taskId, int threadIndex, qint64 bytesReceived,
                        qint64 bytesTotal);
  void stopSelf();
};
