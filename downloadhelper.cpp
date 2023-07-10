#include "downloadhelper.h"

downloadhelper::downloadhelper(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::downloadhelperClass())
{
    ui->setupUi(this);
}

downloadhelper::~downloadhelper()
{
    delete ui;
}
