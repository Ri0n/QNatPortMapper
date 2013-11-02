#ifndef NATPORTMAPPER_WIN_ACTIVEQT_H
#define NATPORTMAPPER_WIN_ACTIVEQT_H

#include <QObject>

#include "natportmapper.h"

class QHostAddress;
class QTimer;
class NatPortMapperPrivate;
namespace NATUPNPLib {
    class UPnPNAT;
    class IStaticPortMappingCollection;
    class IStaticPortMapping;
}

class NatPortMappingActiveQt : public NatPortMapping
{
public:
    NatPortMappingActiveQt(NatPortMapperPrivate *mapper,
                           NATUPNPLib::IStaticPortMapping *mapping,
                           bool autoUnmap = false) :
        NatPortMapping(autoUnmap),
        _mapper(mapper),
        _mapping(mapping) { }
    ~NatPortMappingActiveQt();

    quint16 internalPort() const;
    QHostAddress internalAddress() const;
    quint16 externalPort() const;
    QHostAddress externalAddress() const;
    QString description() const;
    bool unmap();

    QString protoStr() const;
private:
    NatPortMapperPrivate *_mapper;
    NATUPNPLib::IStaticPortMapping *_mapping;
};

class NatPortMapperPrivate : public QObject
{
    Q_OBJECT

    NATUPNPLib::UPnPNAT *_nat;
    NATUPNPLib::IStaticPortMappingCollection *_collection;
    QTimer *_collectionInitTimer;

signals:
    void initialized();

public:
    NatPortMapperPrivate(QObject *parent);
    ~NatPortMapperPrivate();

    inline bool isReady() const { return _collection != NULL; }

    NatPortMapping* add(QAbstractSocket::SocketType socketType,
                int externalPort, int internalPort,
                const QHostAddress &internalAddress, const QString &description);

    bool remove(QAbstractSocket::SocketType socketType, int externalport );    
    bool remove(NatPortMappingActiveQt *mapping);
private slots:
    bool initCollection();
};

#endif // NATPORTMAPPER_WIN_ACTIVEQT_H
