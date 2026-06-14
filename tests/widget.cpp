#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    DebugWindow* dw = new DebugWindow(nullptr);
    dw->show();
    this->close();
}

Widget::~Widget()
{
    delete ui;
}