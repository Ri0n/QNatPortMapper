#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <windows.h>

#include "natportmapper_win_activeqt.h"
#include "natupnp.h"

Q_DECLARE_METATYPE(NatPortMappingActiveQt*)

NatPortMapperPrivate::NatPortMapperPrivate(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<NatPortMappingActiveQt*>();
    _wrapper = new UPnPNATWrapper();
    QThread *t = new QThread(this);
    _wrapper->moveToThread(t);
    connect(_wrapper, SIGNAL(initialized()), SIGNAL(initialized()));
    t->start();
    _wrapper->discover();
}

NatPortMapperPrivate::~NatPortMapperPrivate()
{
    QThread *t = _wrapper->thread();
    t->quit();
    t->wait();
    delete _wrapper;
}

NatPortMapping *NatPortMapperPrivate::add(QAbstractSocket::SocketType socketType,
            int externalPort, int internalPort,
            const QHostAddress &internalAddress, const QString &description)
{
    NatPortMappingActiveQt *mapping = new NatPortMappingActiveQt(_wrapper, socketType, externalPort, internalPort,
                                     internalAddress, description);
    _wrapper->add(mapping);
    return mapping;
}

bool NatPortMapperPrivate::remove(NatPortMappingActiveQt *mapping)
{
    _wrapper->remove(mapping);
    return true;
}



//---------------------------- NatPortMappingActiveQt --------------------------
NatPortMappingActiveQt::~NatPortMappingActiveQt()
{
    if (_autoUnmap) {
        unmap();
    }
}

QHostAddress NatPortMappingActiveQt::externalAddress() const
{
    qDebug() << "FIXME: we can't do this in caller's thread.";
    return QHostAddress(_mapping->ExternalIPAddress());
}

void NatPortMappingActiveQt::unmap()
{
    return _wrapper->remove(this);
}

void NatPortMappingActiveQt::_initFromAdd()
{
    QString proto(_proto == QAbstractSocket::TcpSocket? "TCP": "UDP");
    _mapping = _wrapper->collection()->Add(_externalPort, proto, _internalPort, _internalAddress.toString(),
                  true, _description);
    if (_mapping) {
        QMetaObject::invokeMethod(this, "mapped", Qt::QueuedConnection);
    } else {
        qDebug("AddPortMapping(%hu, %hu) failed",
               _externalPort, _internalPort);
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection);
    }
}

void NatPortMappingActiveQt::_blockingUnmap()
{
    QString proto(_proto == QAbstractSocket::TcpSocket? "TCP": "UDP");
    _wrapper->collection()->Remove(_externalPort, proto);
    delete _mapping;
    _mapping = 0;
    QMetaObject::invokeMethod(this, "unmapped", Qt::QueuedConnection);
}

//-------------------------------------------------------
// UPnPNATWrapper
//-------------------------------------------------------
UPnPNATWrapper::UPnPNATWrapper(QObject *parent) :
    QObject(parent),
    _nat(0),
    _collection(0)
{

}

void UPnPNATWrapper::discover()
{
    QMetaObject::invokeMethod(this, "doDiscover", Qt::QueuedConnection);
}

void UPnPNATWrapper::add(NatPortMappingActiveQt *mapping)
{
    QMetaObject::invokeMethod(this, "doAdd", Qt::QueuedConnection,
                              Q_ARG(NatPortMappingActiveQt*, mapping));
}

void UPnPNATWrapper::remove(NatPortMappingActiveQt *mapping)
{
    QMetaObject::invokeMethod(this, "doRemove", Qt::QueuedConnection,
                              Q_ARG(NatPortMappingActiveQt*, mapping));
}

void UPnPNATWrapper::doDiscover()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _nat = new NATUPNPLib::UPnPNAT(this);
    qDebug() << "nat" << _nat;
    for (int i = 0; i < 3; i++) {
        _collection = _nat->StaticPortMappingCollection();
        qDebug() << "collection" << _nat;
        if (_collection) {
            emit initialized();
            return;
        }
        Sleep(1000);
    }
}

void UPnPNATWrapper::doAdd(NatPortMappingActiveQt *mapping)
{
    if (!_collection) {
        return;
    }
    mapping->_initFromAdd();
}

void UPnPNATWrapper::doRemove(NatPortMappingActiveQt *mapping)
{
    if (!_collection) {
        return;
    }
    mapping->_blockingUnmap();
}
