# ��Ŀ����

## iteration 1

### PSP���

| PSP                                     | Personal Software Process Stages        | Ԥ����ʱ�����ӣ� | ʵ�ʺ�ʱ�����ӣ� |
| --------------------------------------- | --------------------------------------- | ---------------- | ---------------- |
| Planning                                | �ƻ�                                    |        5          |     5             |
| �� Estimate                              | �� �������������Ҫ����ʱ��              |           5       |  5                |
| Development                             | ����                                    |       495           |     750             |
| �� Analysis                              | �� ������� (����ѧϰ�¼���)             |         60    |              120    |
| �� Design Spec                           | �� ��������ĵ�                          |         10         |       5           |
| �� Coding Standard                       | �� ����淶 (ΪĿǰ�Ŀ����ƶ����ʵĹ淶) |          5        |            5      |
| �� Design                                | �� �������                              |    60              |      60            |
| �� Coding                                | �� �������                              |    300              |      500            |
| �� Code Review                           | �� ���븴��                              |  20                |       20           |
| �� Test                                  | �� ���ԣ����Ҳ��ԣ��޸Ĵ��룬�ύ�޸ģ�  |        40          |            40      |
| Reporting                               | ����                                    |             80     |      50            |
| �� Test Report                           | �� ���Ա���                              |       60           |     40             |
| �� Size Measurement                      | �� ���㹤����                            |       10           |    5              |
| �� Postmortem & Process Improvement Plan | �� �º��ܽ�, ��������̸Ľ��ƻ�          |          10        |      5            |
|                                         | �ϼ�                                    |           580       |       805           |

#### ��ʱԭ�����

Qt��Ȼ֮ǰ�������������˲��٣�����֮ǰҲû��ʹ�ù�Qt������⣬���������Ǻ�����.

���߳̿�������Ҫ��һ�������Դ��������ƽʱ���ٽӴ����̱߳�̵ı��ߣ��Ѷ�ʵ�ڲ�С�������Ѷ�����C++�����Ķ�������ʱ���󱨸�ʱ��������Ҫ���Ѻܾò����ҵ���������.

�����Debug��������Ҫ�ȱ��뻷��ռʱ��������ԭ������쳣���ֵ�λ�ã�Qt������Ը����������Ĵ��󶼳�����Qt�⺯���ļ��У�

### ˼·

#### ������Դ

Google + Qt�ĵ� + ChatGPT

#### ���߳�����

���������в��� -> ��ȡ�����ļ����ֽ��� -> ƽ�������ÿ���߳� -> �������� -> �ϲ�

#### ��ʾ����

ʹ��Qt�ṩ��QProgressBar + QNetworkReply��downloadProgress signal

### ʵ�ֹ���

MainWindow�� -> ��ʾ����������

DownloadThread�� -> �̳���QThread�������߳����������ļ�

DownloadCard�� -> �̳���QWidget��������ʾ�����ؽ���

DownloadDetailCard�� -> �̳���QWidget��������ʾÿ���̵߳���������

��Ԫ����ʹ��qDebug���ٵ�����Ϣ

### ���ܷ���

���������ƿ����Ҫ�ڵ����̵߳��ļ�������:

����ļ��ܴ�δ�ӻ���д�뵽�ļ��е��ֽڶ������ڴ��У����³�����ռ�ڴ漫�󣨼���MB��.

�Ľ�˼·��

����һ����ֵ�������̶߳�ȡ���ֽ���������������ֵ���ͽ���ȡ�����ֽ�д���ļ���Ȼ����ϴμ��������������.

��Ȼ��û���������������������Ƶ����IO�����������˿ռ����ܣ�ȴ������ʱ������.

ʹ��visual studio��Performance Profiler�����ܷ��������������ͼ��

![���ܷ���](./imgs/���ܷ���.png)

![��������](./imgs/��������.png)


### ����˵��

������main.cpp������, 
```cpp
#include "mainwindow.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QException>
#include <QString>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QCommandLineParser parser;
  parser.setApplicationDescription("A Download Helper");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption urlOption{{"url", "u"}, "URL to download", "url", ""};
  QCommandLineOption outputOption{{"output", "o"}, "Output filename", "output"};
  QCommandLineOption concurrencyOption{{"concurrency", "n"},
                                       "Concurrency number (default: 8)",
                                       "concurrency",
                                       "8"};

  parser.addOptions({urlOption, outputOption, concurrencyOption});

  parser.process(app);
  const auto url = parser.value(urlOption);
  const auto output = parser.value(outputOption);
  const auto concurrency = parser.value(concurrencyOption).toInt();

  qDebug() << "url = " << url << "\noutput = " << output
           << "\nn = " << concurrency;

  MainWindow mainwindow{};
  mainwindow.show();
  if (url != "") { // ���û������url���Ͳ���Ҫ������������ֱ����ʾ������
    mainwindow.createDownloadTask(url, output, concurrency);
  }
  return app.exec();
}
```

���ȴ���һ��QApplication�����һ��QCommandLineParser����QCommandLineParser�������ڽ��������в�����������������������ѡ�"url"��"output"��"concurrency".

�����������ݸ�����������в������ӽ������м���"url"��"output"��"concurrency"ѡ���ֵ��

Ȼ�󣬳��򴴽�һ��MainWindow������ʾ��������ṩ��"url"ѡ����򽫵���MainWindow�����"createDownloadTask"�����������������м�������"url"��"output"��"concurrency"ֵ���ݸ�����

MainWindow����������£�

```cpp
#pragma once

#include "downloadthread.h"
#include "taskstate.h"
#include "ui_mainwindow.h"
#include "addtaskwindow.h"
#include <QHash>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QSharedPointer>
#include <QFileDialog>
#include <QException>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  /// <summary>
  /// �����������񣬲��������ض���
  /// </summary>
  /// <param name="url">Ҫ�����ļ���url</param>
  /// <param name="output">������ļ���</param>
  /// <param name="concurrency">�߳���</param>
  void createDownloadTask(QString url, QString output, int concurrency);

private:
  Ui::MainWindow *ui;
  QNetworkAccessManager *manager;
  QHash<int, QSharedPointer<TaskState>> tasks;
  int currentTaskId;
  QHash<int, QListWidgetItem *>  listItemsFromTaskId;
  QListWidgetItem *currentItem;
  QHash<QListWidgetItem *, QListWidget *> currentDetail;

private:
  void Download(int taskId, QString url, QString output, int concurrency);
  QString getFilenameFromUrl(QString url);
  QListWidget *createDownloadDetail(int taskId);

signals:
  void taskFinished(int taskId);
private slots:
  void updateTaskState(int taskId, int threadIndex);
  void updateTaskProgressBar(int taskId, int threadIndex, qint64 downloadedSize);
  void updateTaskThreadDetail(int taskId, int threadIndex, qint64 bytesReceived,
                              qint64 bytesTotal);
  /// <summary>
  /// �ϲ�һ��task�ĸ����̵߳��ļ�
  /// </summary>
  /// <param name="taskId">taskId</param>
  void combineFiles(int taskId);
  /// <summary>
  /// �ı����������ʾ����������
  /// </summary>
  /// <param name="item"></param>
  void changeDetail(QListWidgetItem *item);
  void markTaskAsOk(int taskId);
  void showAddTaskWindow();
  void addTask(QString urls, int concurrecny);
  void removeTask(int taskId);
};
```

���У�һЩ��Ҫ�ĺ���ע���Ѿ�����.

�������д������£�

![����](./imgs/����.png)

## Iteration 2

### PSP ���

| PSP                                     | Personal Software Process Stages        | Ԥ����ʱ�����ӣ� | ʵ�ʺ�ʱ�����ӣ� |
| --------------------------------------- | --------------------------------------- | ---------------- | ---------------- |
| Planning                                | �ƻ�                                    |        5          |     5             |
| �� Estimate                              | �� �������������Ҫ����ʱ��              |           5       |  5                |
| Development                             | ����                                    |       700           |     700             |
| �� Analysis                              | �� ������� (����ѧϰ�¼���)             |         60    |              120    |
| �� Design Spec                           | �� ��������ĵ�                          |         10         |       5           |
| �� Coding Standard                       | �� ����淶 (ΪĿǰ�Ŀ����ƶ����ʵĹ淶) |          5        |            5      |
| �� Design                                | �� �������                              |    60              |      60            |
| �� Coding                                | �� �������                              |    300              |      500            |
| �� Code Review                           | �� ���븴��                              |  20                |       20           |
| �� Test                                  | �� ���ԣ����Ҳ��ԣ��޸Ĵ��룬�ύ�޸ģ�  |        40          |            40      |
| Reporting                               | ����                                    |             80     |      50            |
| �� Test Report                           | �� ���Ա���                              |       60           |     40             |
| �� Size Measurement                      | �� ���㹤����                            |       10           |    5              |
| �� Postmortem & Process Improvement Plan | �� �º��ܽ�, ��������̸Ľ��ƻ�          |          10        |      5            |
|                                         | �ϼ�                                    |           785       |       755           |

#### ��ʱ����

���ε����Ѷ����Ĳ����ڡ��ϵ���������Ϊ�˲����˶�����ϲŵ��Խ��.

### ˼·

#### ������Դ

CSDN + Google + Qt�ĵ� + ChatGPT

#### ֹͣ��������

��������Ƽ򵥣�ʵ��ǳ����ӣ������һ��������ɵ������̵߳���quit()������mainwindow���¶�Ӧ������Hash��ɾ��Task������ָ�룻

������ˣ���Ҫ����UI���棬����Ҫ�ر�ע���ָ�������.

���Task��������(reply��ûfinished)��ֹͣ����������൱�����ˣ�ʵ�����£�

```cpp
DownloadThread::~DownloadThread() {
  qDebug() << QString{"thread{%1} exits"}.arg(threadIndex);
  quit();
  wait();
}
void DownloadThread::stop() {
  deleteTemp();
  requestInterruption();
  emit stopSelf();
}
```
����Ҫ��һ���ǡ�ɾ�������صĲ����ļ����������е��߳����ؿ죬�е��߳��������������û����������ֹͣ�����ʱ�����Щ���ؿ���̣߳�part�ļ��Ѿ����浽temp�ļ����ˣ�������Ҫɾ��.

requestInterruption()���ڷ�����ֹ����

stopSelf��һ���Զ����źţ�֪ͨ�����߳���QEventLoop��ֹͣ.

quit()��wait()���ڵȴ��߳��ͷŶ�Ӧ����Դ��ϣ�����reply��Ӧ��һ�룬������߳��Լ��ͷţ�.

#### ��ͣ�Ͷϵ�����

��ͣ���ܺͶϵ������������ƣ�����һ�����.

��DownloadThread.h�У������¹ؼ�˽�г�Ա��

```cpp
Class DownloadThread: public QThread {
  //...
private:
  QString url; // ��¼���ص�url
  qlonglong begin; //���߳̿�ʼ���ص�λ��
  qlonglong end; // ���߳����ص����һ���ֽڣ������䣩
  bool stopped; // �߳���ֹ����
  qint64 pos; //�ϴ���ͣ��pos֮ǰ��ȫ����������
  qint64 newPos; // �������ص�newPos��
  //...
}
```

����pos��newPos������������ʾ�ϴ����ص�λ�úͱ��μ������ص����ֽ�Ϊλ�ã����������߳����ص��ֽ��������ֽ�����ʾΪ��

```cpp
inline qint64 DownloadThread::getBytesDownloaded() { return newPos - begin; }

inline qint64 DownloadThread::getBytesTotal() { return end - begin + 1; }
```

�����½���ʱ����ӣ�

```cpp
void DownloadThread::downloadProgressOfReply(qint64 bytesReceived,
                                             qint64 bytesTotal) {
  newPos = pos + bytesReceived; // �������ص����ֽ�λ��

  qDebug() << QString{"Thread{%1} received{%2}, total{%3}"}
                  .arg(threadIndex)
                  .arg(bytesReceived)
                  .arg(bytesTotal);

  emit downloadSize(taskId, threadIndex, getBytesDownloaded());
  emit downloadProgress(taskId, threadIndex, getBytesDownloaded(),
                        getBytesTotal());
}
```

��������ʱpart�ļ�ʱ����Ϊ

```cpp
void DownloadThread::saveToTempDir(const QByteArray &bytes) {
  auto tempPath = QDir::tempPath();
  auto filename = QUrl(url).fileName() + QString{".part%1"}.arg(threadIndex);
  QFile file{tempPath + "/" + filename};

  // ���֮ǰû��ͣ��
  if (pos == begin) {
    auto success = file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!success) {
      qDebug() << "open " << tempPath + "/" + filename << " failed.";
    }
  } else {
    // ��ͣ������д
    auto success = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (!success) {
      qDebug() << "open " << tempPath + "/" + filename << " failed.";
    }
  }
  auto bytesWritten = file.write(bytes);
  if (bytesWritten == -1) {
    // TODO: д��ʧ��
  }
  file.close();
}
```

���ʵ����ͣʱֹͣreply����.

�ϵ���������ͣ���ƣ�ֻ�����Ǵ����ݿ��ж�ȡÿ���̵߳Ķϵ��Url��Ϣ.

#### ���ض���

����ʹ��UI��ܣ�����֧�����ض��У�����������أ���ʾÿ������Ľ��ȣ�

![���ض���](./imgs/���ض���.png)

#### ����������������

��������������񴰿ڣ����û����"+"ʱ������

![����](./imgs/����.png)

�û������������������ӣ�ʹ��"\n"(����)�ָ�Ϳ���ʵ�ֶ����������.

#### ��Э��

����QNetworkAccessManager������֧�ֶ�Э��ģ����Զ���ftp�ļ�����ʮ�ּ򵥣�ֻ��Ҫ�������߳�����ӣ�

```cpp
if (QUrl(url).scheme() == QLatin1String("ftp")) {
  request.setAttribute(QNetworkRequest::CustomVerbAttribute, "REST 0");
}
```

��δ��븺���ļ����������ļ�ָ����㣬��ʾ��ͷ���أ������Ĳ��������޸ģ�����HttpЭ�鼴��(QtΪ����Э���װ��ͳһ�Ľӿ�).

### ���ܷ���

���ܷ���ͬ�ϴε���������Ͳ�չʾ��.


