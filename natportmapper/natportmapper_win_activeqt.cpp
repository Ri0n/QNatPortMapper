#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include "natportmapper_win_activeqt.h"
#include "natupnp.h"

NatPortMapperPrivate::NatPortMapperPrivate(QObject *parent) :
    QObject(parent), _nat(0), _collection(0),
    _collectionInitTimer(0)
{
    _nat = new NATUPNPLib::UPnPNAT(this);
    // Get the collection of forwarded ports from it, has Windows send UPnP messages to the NAT router
    _collection = _nat->StaticPortMappingCollection();
    if (!_collection) {
        _collectionInitTimer = new QTimer(this);
        _collectionInitTimer->setInterval(1500);
        _collectionInitTimer->setSingleShot(false);
        connect(_collectionInitTimer, SIGNAL(timeout()), SLOT(initCollection()));
        _collectionInitTimer->start();
    }
}

NatPortMapperPrivate::~NatPortMapperPrivate()
{

}

NatPortMapping *NatPortMapperPrivate::add(QAbstractSocket::SocketType socketType,
            int externalPort, int internalPort,
            const QHostAddress &internalAddress, const QString &description)
{
    if (!_collection) {
        return 0;
    }
    QString proto(socketType == QAbstractSocket::TcpSocket? "TCP": "UDP");
    NATUPNPLib::IStaticPortMapping *mapping = _collection
            ->Add(externalPort, proto,internalPort, internalAddress.toString(),
                  true, description);

    return mapping ? new NatPortMappingActiveQt(this, mapping, true) : 0;
}

// Takes a protocol 't' for TCP or 'u' for UDP, and a port being forwarded
// Talks UPnP to the router to remove the forwarding
// Returns false if there was an error
bool NatPortMapperPrivate::remove(QAbstractSocket::SocketType socketType, int externalport )
{
    QString proto(socketType == QAbstractSocket::TcpSocket? "TCP": "UDP");
    _collection->Remove(externalport, proto);
    return true;
}

bool NatPortMapperPrivate::remove(NatPortMappingActiveQt *mapping)
{
    _collection->Remove(mapping->externalPort(), mapping->protoStr());
    return true;
}

bool NatPortMapperPrivate::initCollection()
{
    _collection = _nat->StaticPortMappingCollection();
    if (_collection) {
        _collectionInitTimer->stop();
        emit initialized();
        return true;
    }
    return false;
}

//---------------------------- NatPortMappingActiveQt --------------------------
NatPortMappingActiveQt::~NatPortMappingActiveQt()
{
    if (_autoUnmap) {
        unmap();
    }
}

quint16 NatPortMappingActiveQt::internalPort() const
{
    return _mapping->InternalPort();
}

QHostAddress NatPortMappingActiveQt::internalAddress() const
{
    return QHostAddress(_mapping->InternalClient());
}

quint16 NatPortMappingActiveQt::externalPort() const
{
    return _mapping->ExternalPort();
}

QHostAddress NatPortMappingActiveQt::externalAddress() const
{
    return QHostAddress(_mapping->ExternalIPAddress());
}

QString NatPortMappingActiveQt::description() const
{
    return _mapping->Description();
}

bool NatPortMappingActiveQt::unmap()
{
    return _mapper->remove(this);
}

QString NatPortMappingActiveQt::protoStr() const
{
    return _mapping->Protocol();
}
