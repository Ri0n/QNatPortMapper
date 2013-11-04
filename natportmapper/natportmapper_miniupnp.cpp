#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <QDebug>

#include "natportmapper_miniupnp.h"

class MiniUPnPWrapper : public QObject
{
	Q_OBJECT

	struct IGDdatas *igddatas;
	struct UPNPUrls *upnpurls;
signals:
	void initialized();

public:
	MiniUPnPWrapper(QObject *parent) :
		QObject(parent)
	{
		igddatas = new IGDdatas(); // zero init
		upnpurls = new UPNPUrls(); // zero init
	}

	~MiniUPnPWrapper()
	{
		FreeUPNPUrls(upnpurls);
		delete upnpurls;
		delete igddatas;
	}

	inline bool isInitialized() const {
		return upnpurls->controlURL[0] != '\0';
	}

	void discover() { QMetaObject::invokeMethod(this, "doDiscover", Qt::QueuedConnection); }
	void add(NatPortMappingMiniupnpc *mapping) { QMetaObject::invokeMethod(this, "doAdd", Qt::QueuedConnection, Q_ARG(void*, mapping)); }
private slots:
	void doDiscover()
	{
		struct UPNPDev * devlist;
		struct UPNPDev * dev;
		char * descXML;
		int descXMLsize = 0;
		int upnperror = 0;
		printf("TB : init_upnp()\n");

		devlist = upnpDiscover(2000, NULL/*multicast interface*/, NULL/*minissdpd socket path*/, 0/*sameport*/, 0/*ipv6*/, &upnperror);
		if (devlist)
		{
			dev = devlist;
			while (dev)
			{
				if (strstr (dev->st, "InternetGatewayDevice"))
					break;
				dev = dev->pNext;
			}
			if (!dev)
				dev = devlist; /* defaulting to first device */

			printf("UPnP device :\n"
				   " desc: %s\n st: %s\n",
				   dev->descURL, dev->st);

			descXML = (char*)miniwget(dev->descURL, &descXMLsize, dev->scope_id);
			if (descXML)
			{
				parserootdesc (descXML, descXMLsize, igddatas);
				free (descXML); descXML = 0;
				GetUPNPUrls (upnpurls, igddatas, dev->descURL, dev->scope_id);
			}
			freeUPNPDevlist(devlist);
		}
		else
		{
			/* error ! */
		}
	}

	void doAdd(NatPortMappingMiniupnpc *mapping)
	{
		if(!isInitialized()) {
			qDebug("NPM: the init was not done!");
			return;
		}
		QByteArray port = QByteArray::number(mapping->_port);
		int r = UPNP_AddPortMapping(upnpurls->controlURL, igddatas->first.servicetype,
									QByteArray::number(mapping->_externalPort).constData(),
									QByteArray::number(mapping->_internalPort).constData(),
									mapping->_internalAddress.toString().toLatin1(),
									mapping->_description, mapping->protoStr().toLatin1().constData(), NULL, NULL);
		if(r==0)
			printf("AddPortMapping(%s, %s, %s) failed\n", port_str, port_str, addr);
	}
};

NatPortMapperPrivate::NatPortMapperPrivate(QObject *parent) :
	QObject(parent), _wrapper(0)
{
	_wrapper = new MiniUPnPWrapper(this);
	QThread *t = new QThread(this);
	_wrapper->moveToThread(t);
	connect(_wrapper, SIGNAL(initialized()), SIGNAL(initialized()));
	t->start();
}

NatPortMapperPrivate::~NatPortMapperPrivate()
{
	QThread *t = _wrapper->thread();
	t->quit();
	t->wait();
}

bool NatPortMapperPrivate::isReady() const
{
	return _wrapper->isInitialized();
}

NatPortMapping *NatPortMapperPrivate::add(QAbstractSocket::SocketType socketType,
			int externalPort, int internalPort,
			const QHostAddress &internalAddress, const QString &description)
{

	NatPortMappingMiniupnpc *mapping = new NatPortMappingMiniupnpc(this, socketType, externalPort, internalPort,
																   internalAddress, description);
	_wrapper->add(mapping);
	return mapping;
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

bool NatPortMapperPrivate::remove(NatPortMappingMiniupnpc *mapping)
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

//---------------------------- NatPortMappingMiniupnpc --------------------------
NatPortMappingMiniupnpc::~NatPortMappingMiniupnpc()
{
	if (autoUnmap) {
		unmap();
	}
}

quint16 NatPortMappingMiniupnpc::internalPort() const
{
	return _mapping->InternalPort();
}

QHostAddress NatPortMappingMiniupnpc::internalAddress() const
{
	return QHostAddress(_mapping->InternalClient());
}

quint16 NatPortMappingMiniupnpc::externalPort() const
{
	return _mapping->ExternalPort();
}

QHostAddress NatPortMappingMiniupnpc::externalAddress() const
{
	return QHostAddress(_mapping->ExternalIPAddress());
}

QString NatPortMappingMiniupnpc::description() const
{
	return _mapping->Description();
}

bool NatPortMappingMiniupnpc::unmap()
{
	return _mapper->remove(this);
}

QString NatPortMappingMiniupnpc::protoStr() const
{
	return _mapping->Protocol();
}
