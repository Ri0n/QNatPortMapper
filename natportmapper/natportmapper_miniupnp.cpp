#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <QDebug>

#include "natportmapper_miniupnp.h"

Q_DECLARE_METATYPE(NatPortMappingMiniupnpc*)

class MiniUPnPWrapper : public QObject
{
    Q_OBJECT

    struct IGDdatas *igddatas;
    struct UPNPUrls *upnpurls;
    QHostAddress _externalAddress;
signals:
    void initialized();

public:
    MiniUPnPWrapper(QObject *parent = 0) :
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
        return upnpurls->controlURL != 0;
    }

    inline QHostAddress externalAddress() const { return _externalAddress; }

    void discover() { QMetaObject::invokeMethod(this, "doDiscover", Qt::QueuedConnection); }
    void add(NatPortMappingMiniupnpc *mapping) { QMetaObject::invokeMethod(this, "doAdd", Qt::QueuedConnection, Q_ARG(NatPortMappingMiniupnpc*, mapping)); }
    void remove(NatPortMappingMiniupnpc *mapping) { QMetaObject::invokeMethod(this, "doRemove", Qt::QueuedConnection, Q_ARG(NatPortMappingMiniupnpc*, mapping)); }
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

            QByteArray ipBuffer;
            ipBuffer.resize(16);
            if (UPNP_GetExternalIPAddress(upnpurls->controlURL, igddatas->first.servicetype,
                                          ipBuffer.data()) == 0) {
                _externalAddress = QHostAddress(QString::fromLatin1(ipBuffer.constData()));
            }

            freeUPNPDevlist(devlist);
            emit initialized();
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
        int r = UPNP_AddPortMapping(upnpurls->controlURL, igddatas->first.servicetype,
                                    QByteArray::number(mapping->_externalPort).constData(),
                                    QByteArray::number(mapping->_internalPort).constData(),
                                    mapping->_internalAddress.toString().toLatin1(),
                                    mapping->_description.toUtf8().constData(),
                                    mapping->protoStr().toLatin1().constData(), NULL, NULL);
        if (r != 0) {
            qDebug("AddPortMapping(%hu, %hu) failed",
                   mapping->externalPort(), mapping->internalPort());
            QMetaObject::invokeMethod(mapping, "error", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(mapping, "mapped", Qt::QueuedConnection);
        }
    }

    void doRemove(NatPortMappingMiniupnpc *mapping)
    {
        if(!isInitialized()) {
            qDebug("NPM: the init was not done!");
            return;
        }
        int r = UPNP_DeletePortMapping(upnpurls->controlURL, igddatas->first.servicetype,
                                       QByteArray::number(mapping->_externalPort).constData(),
                                       mapping->protoStr().toLatin1().constData(), NULL);
        if (r != 0) {
            qDebug("DeletePortMapping(%hu) failed", mapping->externalPort());
            QMetaObject::invokeMethod(mapping, "error", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(mapping, "unmapped", Qt::QueuedConnection);
        }
    }
};

NatPortMapperPrivate::NatPortMapperPrivate(QObject *parent) :
    QObject(parent), _wrapper(0)
{
    qRegisterMetaType<NatPortMappingMiniupnpc*>();
    _wrapper = new MiniUPnPWrapper();
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

bool NatPortMapperPrivate::isReady() const
{
    return _wrapper->isInitialized();
}

NatPortMapping *NatPortMapperPrivate::add(QAbstractSocket::SocketType socketType,
            int externalPort, int internalPort,
            const QHostAddress &internalAddress, const QString &description)
{

    NatPortMappingMiniupnpc *mapping = new NatPortMappingMiniupnpc(_wrapper, socketType, externalPort, internalPort,
                                     internalAddress, description);
    _wrapper->add(mapping);
    return mapping;
}

//---------------------------- NatPortMappingMiniupnpc --------------------------
NatPortMappingMiniupnpc::~NatPortMappingMiniupnpc()
{
    if (_autoUnmap) {
        unmap();
    }
}

quint16 NatPortMappingMiniupnpc::internalPort() const
{
    return _internalPort;
}

QHostAddress NatPortMappingMiniupnpc::internalAddress() const
{
    return _internalAddress;
}

quint16 NatPortMappingMiniupnpc::externalPort() const
{
    return _externalPort;
}

QHostAddress NatPortMappingMiniupnpc::externalAddress() const
{
    return _wrapper->externalAddress();
}

QString NatPortMappingMiniupnpc::description() const
{
    return _description;
}

void NatPortMappingMiniupnpc::unmap()
{
    _wrapper->remove(this);
}

QString NatPortMappingMiniupnpc::protoStr() const
{
    return QString(_proto == QAbstractSocket::TcpSocket? "TCP": "UDP");
}

#include "natportmapper_miniupnp.moc"
