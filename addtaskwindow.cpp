#include "addtaskwindow.h"

AddTaskWindow::AddTaskWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AddTaskWindowClass()) {
  ui->setupUi(this);
  connect(ui->add, &QPushButton::clicked, [this]() {
    emit addTask(ui->urls->toPlainText(), ui->concurrency->value());
    this->close();
  });
}

AddTaskWindow::~AddTaskWindow() { delete ui; }
