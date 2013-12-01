#ifndef NATPORTMAPPER_MINIUPNP_H
#define NATPORTMAPPER_MINIUPNP_H

#include <QHostAddress>

#include "natportmapper.h"

class QHostAddress;
class QTimer;
class MiniUPnPWrapper;

class NatPortMappingMiniupnpc : public NatPortMapping
{
    Q_OBJECT
public:
    NatPortMappingMiniupnpc(MiniUPnPWrapper *wrapper, QAbstractSocket::SocketType socketType,
							quint16 externalPort, quint16 internalPort,
							const QHostAddress &internalAddress, const QString &description) :
		_wrapper(wrapper),
		_proto(socketType),
		_externalPort(externalPort),
		_internalPort(internalPort),
		_internalAddress(internalAddress),
		_description(description)
	{}

    ~NatPortMappingMiniupnpc();

	quint16 internalPort() const;
	QHostAddress internalAddress() const;
	quint16 externalPort() const;
	QHostAddress externalAddress() const;
	QString description() const;
    void unmap();

	QString protoStr() const;
private:
	friend class MiniUPnPWrapper;
	MiniUPnPWrapper *_wrapper;
	QAbstractSocket::SocketType _proto;
	quint16 _externalPort;
	quint16 _internalPort;
	QHostAddress _internalAddress;
	QHostAddress _externalAddress;
	QString _description;

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
};

#endif
