#ifndef NATPORTMAPPER_WIN_ACTIVEQT_H
#define NATPORTMAPPER_WIN_ACTIVEQT_H

#include <QObject>
#include <QHostAddress>

#include "natportmapper.h"

class QHostAddress;
class QTimer;
class NatPortMapperPrivate;
class UPnPNATWrapper;
namespace NATUPNPLib {
    class UPnPNAT;
    class IStaticPortMappingCollection;
    class IStaticPortMapping;
}

class NatPortMappingActiveQt : public NatPortMapping
{
public:
    NatPortMappingActiveQt(UPnPNATWrapper *wrapper,
                           NATUPNPLib::IStaticPortMapping *mapping) :
        _wrapper(wrapper),
        _mapping(mapping) { }
    ~NatPortMappingActiveQt();

    NatPortMappingActiveQt(UPnPNATWrapper *wrapper, QAbstractSocket::SocketType socketType,
                           int externalPort, int internalPort,
                           const QHostAddress &internalAddress, const QString &description) :
        NatPortMapping(socketType, externalPort, internalPort, internalAddress, description),
        _wrapper(wrapper),
        _mapping(0) { }


    QHostAddress externalAddress() const;
    void unmap();
private:
    friend class UPnPNATWrapper;

    // next function called from wrapper thread
    void _initFromAdd();
    void _blockingUnmap();

    UPnPNATWrapper *_wrapper;
    NATUPNPLib::IStaticPortMapping *_mapping;
};


class UPnPNATWrapper : public QObject
{
    Q_OBJECT

    NATUPNPLib::UPnPNAT *_nat;
    NATUPNPLib::IStaticPortMappingCollection *_collection;
public:
    UPnPNATWrapper(QObject *parent = 0);

    void discover();
    void add(NatPortMappingActiveQt *mapping);
    void remove(NatPortMappingActiveQt *mapping);

    inline bool isInitialized() const {
        return _collection != 0;
    }
    inline NATUPNPLib::IStaticPortMappingCollection *collection() const { return _collection; }

signals:
    void initialized();

private slots:
    void doDiscover();
    void doAdd(NatPortMappingActiveQt *mapping);
    void doRemove(NatPortMappingActiveQt *mapping);
};


class NatPortMapperPrivate : public QObject
{
    Q_OBJECT

    UPnPNATWrapper *_wrapper;

signals:
    void initialized();

public:
    NatPortMapperPrivate(QObject *parent);
    ~NatPortMapperPrivate();

    inline bool isReady() const { return _wrapper->isInitialized(); }

    NatPortMapping* add(QAbstractSocket::SocketType socketType,
                int externalPort, int internalPort,
                const QHostAddress &internalAddress, const QString &description);

    bool remove(NatPortMappingActiveQt *mapping);
};

#endif // NATPORTMAPPER_WIN_ACTIVEQT_H
