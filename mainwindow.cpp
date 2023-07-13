#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0),
      taskFromItem(), currentItem(nullptr) {
  ui->setupUi(this);
  connect(ui->downloadingList, &QListWidget::itemClicked, this,
          &MainWindow::changeDetail);
  connect(this, &MainWindow::taskFinished, this, &MainWindow::markTaskAsOk);
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
        int threadNum = concurrency;
        if (reply->error() != QNetworkReply::NoError) {
          // TODO: 出现请求错误
        }
        auto contentLength =
            reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();

        // 判断是否支持多线程
        QEventLoop loop{};
        QNetworkRequest request{QUrl(url)};
        request.setRawHeader("Range", "bytes=0-0");
        auto testReply = this->manager->get(request);
        connect(testReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        // 如果响应码为206, 表示支持多线程
        auto statusCode =
            testReply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                .toInt();
        qDebug() << "statusCode = " << statusCode;
        if (statusCode != 206) {
          // 不支持就使用单线程
          threadNum = 1;
          qDebug() << "not support multi(?)thread";
        }

        qDebug() << "concurrency = " << threadNum;

        auto blockSize = contentLength / threadNum;

        qDebug() << "content-length = " << contentLength;

        auto task = tasks[taskId];

        // 设置TaskState的总字节数
        task->setBytesTotal(contentLength);

        auto threads = task->getThreads();
        for (int i = 0; i < threadNum; ++i) {
          auto begin = i * blockSize;
          auto end = begin + blockSize - 1;
          if (i == threadNum - 1) {
            end = contentLength - 1;
          }
          auto thread =
              new DownloadThread(taskId, i, url, begin, end,
                                 task.get()); // TODO: 给线程添加ProgressBar
          connect(thread, &DownloadThread::downloadSize, this,
                  &MainWindow::updateTaskProgressBar);
          connect(thread, &DownloadThread::downloadFinished, this,
                  &MainWindow::updateTaskState);
          connect(thread, &DownloadThread::downloadProgress, this,
                  &MainWindow::updateTaskThreadDetail);
          threads.push_back(thread);
          thread->start();
        }
      });
  // TODO: connect 本线程下载完毕

  connect(this, &MainWindow::taskFinished, this, &MainWindow::combineFiles);
}

QString MainWindow::getFilenameFromUrl(QString url) {
  return QUrl(url).fileName();
}

void MainWindow::updateTaskProgressBar(int taskId, int threadIndex,
                                       qint64 downloadedSize) {
  auto task = tasks[taskId];
  task->updateProgressBar(threadIndex, downloadedSize);
}

void MainWindow::updateTaskThreadDetail(int taskId, int threadIndex,
                                        qint64 bytesReceived,
                                        qint64 bytesTotal) {
  auto task = tasks[taskId];
  task->updateDetailCard(threadIndex, bytesReceived, bytesTotal);
}

void MainWindow::combineFiles(int taskId) {
  auto task = tasks[taskId];
  task->combine();
}

void MainWindow::createDownloadTask(QString url, QString output,
                                    int concurrency) {

  QListWidgetItem *listItem = new QListWidgetItem(ui->downloadingList);
  ui->downloadingList->addItem(listItem);

  DownloadCard *downloadCard = new DownloadCard(getFilenameFromUrl(url));
  listItem->setSizeHint(downloadCard->size());
  ui->downloadingList->setItemWidget(listItem, downloadCard);
  downloadCard->show();

  // ui->downloadingList->addItem("dsadsa");

  QHash<int, bool> threadState{};
  for (int i = 0; i < concurrency; ++i) {
    threadState.insert(i, false);
  }

  auto task =
      QSharedPointer<TaskState>{new TaskState(downloadCard, threadState, this)};
  tasks.insert(currentTaskId, task);
  taskFromItem.insert(listItem, currentTaskId);
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

void MainWindow::changeDetail(QListWidgetItem *item) {
  qDebug() << taskFromItem[item] << " is selected.";

  // 多次点击同一个item不会更改detail视图
  if (currentItem == item) {
    return;
  }
  currentItem = item;

  try {
    auto task = tasks[taskFromItem[item]];
    auto downloadDetail = ui->downloadDetail;
    auto details = task->getDetails();

    qDebug() << QString{"task = {%1}, downloadDetail = {%2}, details = {%3}"}
                    .arg(qint64(task.get()))
                    .arg(qint64(downloadDetail));
    qDebug() << "details = " << details;

    for (auto detail : details) {
      auto listWidget = new QListWidgetItem(downloadDetail);
      downloadDetail->addItem(listWidget);

      listWidget->setSizeHint(detail->size());
      downloadDetail->setItemWidget(listWidget, detail);
    }
  } catch (std::exception e) {
    qDebug() << e.what();
  }
}

void MainWindow::markTaskAsOk(int taskId) {
  auto task = tasks[taskId];
  task->setOkVisible();
}
