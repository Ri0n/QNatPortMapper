#ifndef NATPORTMAPPER_MINIUPNP_H
#define NATPORTMAPPER_MINIUPNP_H

#include "natportmapper.h"

class QHostAddress;
class QTimer;
class MiniUPnPWrapper;

class NatPortMappingMiniupnpc : public NatPortMapping
{
public:
	NatPortMappingMiniupnpc(NatPortMapperPrivate *mapper,
						   bool autoUnmap = false) :
		NatPortMapping(autoUnmap),
		_mapper(mapper) { }
	~NatPortMappingMiniupnpc();

	quint16 internalPort() const;
	QHostAddress internalAddress() const;
	quint16 externalPort() const;
	QHostAddress externalAddress() const;
	QString description() const;
	bool unmap();

	QString protoStr() const;
private:
	friend class MiniUPnPWrapper;
	NatPortMapperPrivate *_mapper;

};

class NatPortMapperPrivate : public QObject
{
	Q_OBJECT

	MiniUPnPWrapper *_wrapper;

signals:
	void initialized();

public:
	NatPortMapperPrivate(QObject *parent);
	~NatPortMapperPrivate();

	bool isReady() const;

	NatPortMapping* add(QAbstractSocket::SocketType socketType,
				int externalPort, int internalPort,
				const QHostAddress &internalAddress, const QString &name);

	bool remove(QAbstractSocket::SocketType socketType, int externalport );
	bool remove(NatPortMappingMiniupnpc *mapping);
private slots:
	bool initCollection();
};

#endif
