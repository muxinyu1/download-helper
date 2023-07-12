#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0) {
  ui->setupUi(this);
}

MainWindow::MainWindow(QString url, QString output, int concurrency,
                       QWidget *parrent)
    : QMainWindow(parrent), ui(new Ui::MainWindow()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0) {
  ui->setupUi(this);
  createDownloadTask(url, output, concurrency);
}

MainWindow::~MainWindow() {
  delete ui;
  delete manager;
}

void MainWindow::Download(int taskId, QString url, QString output,
                          int concurrency) {
  qDebug() << QString{"create task{%1}, url = {%2}"}.arg(taskId).arg(url);
  QNetworkRequest request{QUrl(url)};
  auto reply = this->manager->head(request);
  connect(
      reply, &QNetworkReply::finished, this,
      [this, reply, concurrency, url, taskId]() {
        if (reply->error() != QNetworkReply::NoError) {
          // TODO: 出现请求错误
        }
        auto contentLength =
            reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        auto blockSize = contentLength / concurrency;

        qDebug() << "content-length = " << contentLength;

        // 设置TaskState的总字节数
        tasks[taskId]->setBytesTotal(contentLength);

        // 初始化task对应的QHash
        auto &threadState = tasks[taskId]->getThreadState();
        for (int i = 0; i < concurrency; ++i) {
          threadState.insert(i, false);
        }

        auto threads = tasks[taskId]->getThreads();
        for (int i = 0; i < concurrency; ++i) {
          auto begin = i * blockSize;
          auto end = begin + blockSize - 1;
          if (i == concurrency - 1) {
            end = contentLength - 1;
          }
          auto thread = new DownloadThread(
              taskId, i, url, begin, end, nullptr, nullptr); // TODO: 给线程添加ProgressBar
          connect(thread, &DownloadThread::downloadSize, this,
                  &MainWindow::updateTaskProgressBar);
          connect(thread, &DownloadThread::downloadFinished, this,
                  &MainWindow::updateTaskState);
          threads.push_back(thread);
          thread->start();
        }
      });
  // TODO: connect 本线程下载完毕

  connect(this, &MainWindow::taskFinished, this, &MainWindow::combineFiles);
}

void MainWindow::updateTaskProgressBar(int taskId, int threadIndex, qint64 downloadedSize) {
  auto task = tasks[taskId];
  task->updateProgressBar(threadIndex, downloadedSize);
}

void MainWindow::combineFiles(int taskId) {
  auto task = tasks[taskId];
  task->combine();
}

void MainWindow::createDownloadTask(QString url, QString output,
                                    int concurrency) {
  
  QListWidgetItem *listItem = new QListWidgetItem(ui->downloadingList);
  ui->downloadingList->addItem(listItem);

  DownloadCard *downloadCard = new DownloadCard();
  listItem->setSizeHint(downloadCard->size());
  ui->downloadingList->setItemWidget(listItem, downloadCard);
  downloadCard->show();

  // ui->downloadingList->addItem("dsadsa");

  tasks.insert(currentTaskId, QSharedPointer<TaskState>{new TaskState(
                                  downloadCard, QHash<int, bool>{}, this)});
  Download(currentTaskId, url, output, concurrency);
  ++currentTaskId;
}

void MainWindow::updateTaskState(int taskId, int threadIndex) {
  auto task = tasks[taskId];
  task->getThreadState()[threadIndex] = true;

  qDebug() << QString{"MainWindow set taskId = %1, threadIndex = %2, true"}
                  .arg(taskId)
                  .arg(threadIndex);

  bool finished = true;
  for (auto i = task->getThreadState().begin();
       i != task->getThreadState().end(); ++i) {
    if (!*i) {
      finished = false;
      break;
    }
  }
  if (finished) {
    qDebug() << "All threads ok, start to combine...";

    emit taskFinished(taskId);
  }
}
