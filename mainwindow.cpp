#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "natportmapper.h"
#include <QHostAddress>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mapper(new NatPortMapper(this))
{
    ui->setupUi(this);
    if (mapper->isReady()) {
        mapperReady();
    } else {
        connect(mapper, SIGNAL(initialized()), SLOT(mapperReady()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mapperReady()
{
    ui->lblStatus->setText("Initialized!!!");
    NatPortMapping *mapping = mapper->map(QAbstractSocket::TcpSocket, 8910, 8910, QHostAddress(QString("192.168.1.2")), "upnptest");
    ui->lblInternal->setText(QString("%1:%2").arg(mapping->internalAddress().toString()).arg(mapping->internalPort()));
    ui->lblExternal->setText(QString("%1:%2").arg(mapping->externalAddress().toString()).arg(mapping->externalPort()));
    ui->lblDesc->setText(mapping->description());
}
