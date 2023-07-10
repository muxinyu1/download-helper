#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0) {
  ui->setupUi(this);
}

MainWindow::MainWindow(QString url, QString output, int concurrency,
                       QWidget *parrent)
    : QMainWindow(parrent), ui(new Ui::MainWindowClass()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0) {
  ui->setupUi(this);
  createDownloadTask(url, output, concurrency);
}

MainWindow::~MainWindow() {
  delete ui;
  delete manager;
}

void MainWindow::Download(int taskId, QString url, QString output, int concurrency) {
  QNetworkRequest request{QUrl(url)};
  auto reply = this->manager->head(request);
  connect(
      reply, &QNetworkReply::isFinished, this,
      [this, reply, concurrency, url, taskId]() {
        if (reply->error() != QNetworkReply::NoError) {
          // TODO: 出现请求错误
        }
        auto contentLength =
            reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        auto blockSize = contentLength / concurrency;

        // 初始化task对应的QHash
        for (int i = 0; i < concurrency; ++i) {
          tasks[taskId].insert(i, false);
        }

        QList<DownloadThread *> threads{};
        for (int i = 0; i < concurrency; ++i) {
          auto begin = i * blockSize;
          auto end = begin + blockSize - 1;
          if (i == concurrency - 1) {
            end = contentLength - 1;
          }
          DownloadThread *thread =
              new DownloadThread(taskId, i, url, begin, end, , this);
          connect(thread, &DownloadThread::downloadFinished, this,
                  &MainWindow::updateTaskState);
          threads.push_back(thread);
          thread->start();
        }
      });
  // TODO: connect 本线程下载完毕
}

void MainWindow::createDownloadTask(QString url, QString output,
                                    int concurrency) {
  tasks.insert(currentTaskId, QHash<int, bool>{});
  Download(currentTaskId, url, output, concurrency);
  ++currentTaskId;
}

void MainWindow::updateTaskState(int taskId, int threadIndex) {
  tasks[taskId][threadIndex] = true;
  auto &task = tasks[taskId];
  bool finished = true;
  for (auto i = task.begin(); i != task.end(); ++i) {
    if (!*i) {
      finished = false;
      break;
    }
  }
  if (finished) {
    emit taskFinished(taskId);
  }
}
