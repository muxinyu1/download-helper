#pragma once

#include "ui_addtaskwindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class AddTaskWindowClass;
};
QT_END_NAMESPACE

class AddTaskWindow : public QMainWindow {
  Q_OBJECT

public:
  AddTaskWindow(QWidget *parent = nullptr);
  ~AddTaskWindow();
signals:
  void addTask(QString urls, int concurrency, bool speedLimit, int speed);

private:
  Ui::AddTaskWindowClass *ui;
};
