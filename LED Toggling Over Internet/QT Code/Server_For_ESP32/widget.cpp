#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->pushButton_2->setEnabled(false);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    server.listen(QHostAddress("192.168.1.44"),1881);
    if(server.isListening()) {
        connect(&this->server, &QTcpServer::newConnection, this, &Widget::newConnection);
        ui->pushButton->setText("Server Started");
    }
}

void Widget::newConnection()
{
    ui->pushButton_2->setEnabled(true);
    this->incomingSocket = server.nextPendingConnection();
}

void Widget::on_pushButton_2_clicked()
{
    this->incomingSocket->write("Toggle");
}

