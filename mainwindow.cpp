#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow()),
      manager(new QNetworkAccessManager()), tasks(), currentTaskId(0),
      listItemsFromTaskId(), currentItem(nullptr), currentDetail() {
  ui->setupUi(this);
  connect(ui->downloadingList, &QListWidget::itemClicked, this,
          &MainWindow::changeDetail);
  connect(this, &MainWindow::taskFinished, this, &MainWindow::markTaskAsOk);
  connect(ui->add, &QPushButton::clicked, this, &MainWindow::showAddTaskWindow);
  connect(this, &MainWindow::taskFinished, this, &MainWindow::combineFiles);
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
          task->addThread(thread);
          thread->start();
        }
      });
  // TODO: connect 本线程下载完毕
}

QString MainWindow::getFilenameFromUrl(QString url) {
  return QUrl(url).fileName();
}

QListWidget *MainWindow::createDownloadDetail(int taskId) {
  auto task = tasks[taskId];
  auto &details = task->getDetails();
  auto downloadDetail = new QListWidget(ui->frame);
  downloadDetail->setGeometry(ui->downloadDetail->geometry());
  downloadDetail->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  downloadDetail->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  downloadDetail->setMovement(QListView::Static);
  downloadDetail->setFlow(QListView::TopToBottom);
  downloadDetail->setResizeMode(QListView::Adjust);
  downloadDetail->setViewMode(QListView::IconMode);

  // 添加表项
  for (auto detail : details) {
    auto listItem = new QListWidgetItem(downloadDetail);
    downloadDetail->addItem(listItem);

    listItem->setSizeHint(detail->size());
    downloadDetail->setItemWidget(listItem, detail);
  }
  downloadDetail->hide();
  return downloadDetail;
}

QString MainWindow::getSavedDir(const QString& filename) { 

QString ext = QFileInfo(filename).suffix().toLower();
  QString dir;

  if (ext.isEmpty()) { // 没有后缀名
    dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  } else if (ext == "txt" || ext == "md" || ext == "doc" || ext == "docx" ||
             ext == "odt") { // 文本文件
    dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  } else if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "bmp" ||
             ext == "gif") { // 图片文件
    dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
  } else if (ext == "mp3" || ext == "wav" || ext == "flac" || ext == "aac" ||
             ext == "wma") { // 音乐文件
    dir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
  } else if (ext == "mp4" || ext == "avi" || ext == "mov" || ext == "mkv" ||
             ext == "wmv") { // 视频文件
    dir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
  } else if (ext == "pdf") { // PDF文件
    dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  } else if (ext == "zip" || ext == "rar" || ext == "7z" || ext == "tar.gz" ||
             ext == "tgz") { // 压缩文件
    dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  } else { // 没有匹配的后缀名
    dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  }

  return dir;
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
  auto savedDir = getSavedDir(task->getFilename());
  auto caption =
      QString{"Select A Position to Save %1"}.arg(task->getFilename());
  auto savedDir = QFileDialog::getExistingDirectory(this, caption, savedDir);
  int threadNum = task->getThreadNum();
  auto filename = task->getFilename();
  QFile file{savedDir + "/" + filename};
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "open " << savedDir + "/" + filename << " failed.";
    file.close();
    return;
  }

  qDebug() << "start to write " << savedDir + "/" + filename
           << ", threadNum = " << threadNum;

  // 组合文件
  for (decltype(threadNum) i = 0; i < threadNum; ++i) {
    auto tempDir = QDir::tempPath();
    auto partFilename =
        tempDir + "/" + QString{"%1.part%2"}.arg(filename).arg(i);
    QFile partFile{partFilename};
    if (!partFile.open(QIODevice::ReadOnly)) {
      qDebug() << "open " << partFilename << " failed.";
      file.close();
      partFile.close();
      return;
    }
    file.write(partFile.readAll());

    qDebug() << partFilename << " has already been written to "
             << savedDir + "/" + filename;

    // 删除temp文件
    try {
      partFile.remove();
      partFile.close();
    } catch (QException e) {
      qDebug() << "remove " << partFilename << " error: " << e.what();
    }
  }
  qDebug() << filename << " saved.";
  file.close();
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

  auto task = QSharedPointer<TaskState>{
      new TaskState(currentTaskId, downloadCard, threadState, QUrl{url}.fileName(), this)};
  tasks.insert(currentTaskId, task);
  listItemsFromTaskId.insert(currentTaskId, listItem);
  Download(currentTaskId, url, output, concurrency);

  currentDetail.insert(listItem, createDownloadDetail(currentTaskId));

  ++currentTaskId;

  connect(task.get(), &TaskState::remove, this, &MainWindow::removeTask);
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
  qDebug() << item << " is selected.";

  // 多次点击同一个item不会更改detail视图
  if (currentItem == item) {
    return;
  }
  if (currentItem != nullptr) {
    currentDetail[currentItem]->hide();
  }
  currentDetail[item]->show();
  currentItem = item;
  /*try {
    auto task = tasks[taskFromItem[item]];
    auto downloadDetail = ui->downloadDetail;
    auto details = task->getDetails();

    qDebug() << QString{"task = {%1}, downloadDetail = {%2}, details = {%3}"}
                    .arg(qint64(task.get()))
                    .arg(qint64(downloadDetail));
    qDebug() << "details = " << details;

    for (int i = 0; i < downloadDetail->count(); ++i) {
      downloadDetail->takeItem(i);
    }

    for (auto detail : details) {
      auto listWidget = new QListWidgetItem(downloadDetail);
      downloadDetail->addItem(listWidget);

      listWidget->setSizeHint(detail->size());
      downloadDetail->setItemWidget(listWidget, detail);
    }
  } catch (std::exception e) {
    qDebug() << e.what();
  }*/
}

void MainWindow::markTaskAsOk(int taskId) {
  auto task = tasks[taskId];
  task->setOkVisible();
}

void MainWindow::showAddTaskWindow() {
  auto addTaskWindow = new AddTaskWindow(this);
  connect(addTaskWindow, &AddTaskWindow::addTask, this, &MainWindow::addTask);
  addTaskWindow->show();
}

void MainWindow::addTask(QString urls, int concurrency) {
  auto urlList = urls.split('\n');
  for (auto &url : urlList) {
    createDownloadTask(url, "", concurrency);
  }
}

void MainWindow::removeTask(int taskId) {
  auto task = tasks[taskId];
  auto listItem = listItemsFromTaskId[taskId];
  auto downloadDetail = currentDetail[listItem];
  downloadDetail->close();
  auto row = ui->downloadingList->row(listItem);
  auto p = ui->downloadingList->takeItem(row);
  delete p;

  currentDetail.remove(listItem);
  listItemsFromTaskId.remove(taskId);
  tasks.remove(taskId);
  if (currentItem == listItem) {
    currentItem = nullptr;
  }
}
