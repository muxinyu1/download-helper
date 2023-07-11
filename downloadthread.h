#pragma once

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QThread>

class DownloadThread : public QThread {
  Q_OBJECT

public:
  DownloadThread(int taskId, int threadIndex, QString url, qlonglong begin,
                 qlonglong end, QProgressBar *progressBar,
                 QObject *parrent = nullptr);
  ~DownloadThread();

protected:
  void run() override;

private:
  int taskId;
  int threadIndex;
  QString url;
  qlonglong begin;
  qlonglong end;
  QProgressBar *progressBar;

  void downloadPart();
  void saveToTempDir(const QByteArray &bytes);

signals:
  void downloadFinished(int taskId, int threadIndex);
  void downloadSize(int taskId, qint64 downloadedSize);
};
