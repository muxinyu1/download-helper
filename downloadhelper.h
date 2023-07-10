#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_downloadhelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class downloadhelperClass; };
QT_END_NAMESPACE

class downloadhelper : public QMainWindow
{
    Q_OBJECT

public:
    downloadhelper(QWidget *parent = nullptr);
    ~downloadhelper();

private:
    Ui::downloadhelperClass *ui;
};
