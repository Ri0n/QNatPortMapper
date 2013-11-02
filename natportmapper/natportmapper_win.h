#ifndef NATPORTMAPPER_WIN_H
#define NATPORTMAPPER_WIN_H

#include <objbase.h>

#include "natportmapper.h"

class QHostAddress;
class QTimer;

typedef interface IUPnPNAT IUPnPNAT;
typedef interface IStaticPortMappingCollection IStaticPortMappingCollection;
typedef interface IStaticPortMapping IStaticPortMapping;

class NatPortMappingWin : public NatPortMapping
{
public:
    NatPortMappingWin(NatPortMapperPrivate *mapper,
                           IStaticPortMapping *mapping,
                           bool autoUnmap = false) :
        NatPortMapping(autoUnmap),
        _mapper(mapper),
        _mapping(mapping) { }
    ~NatPortMappingWin();

    quint16 internalPort() const;
    QHostAddress internalAddress() const;
    quint16 externalPort() const;
    QHostAddress externalAddress() const;
    QString description() const;
    bool unmap();

    QString protoStr() const;
private:
    NatPortMapperPrivate *_mapper;
    IStaticPortMapping *_mapping;
};

class NatPortMapperPrivate : public QObject
{
    Q_OBJECT

    IUPnPNAT*                     _nat;
    IStaticPortMappingCollection* _collection;
    QTimer *_collectionInitTimer;

signals:
    void initialized();

public:
    NatPortMapperPrivate(QObject *parent);
    ~NatPortMapperPrivate();

    inline bool isReady() const { return _collection != NULL; }

    NatPortMappingWin* add(QAbstractSocket::SocketType socketType,
                int externalPort, int internalPort,
                const QHostAddress &internalAddress, const QString &name);

    bool remove(QAbstractSocket::SocketType socketType, int externalport );    
    bool remove(NatPortMappingWin *mapping);
private slots:
    bool initCollection();
};

#endif // NATPORTMAPPER_WIN_H
