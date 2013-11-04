#include <QTimer>

#include "natportmapper.h"
#ifdef Q_OS_WIN
# ifdef NPM_ACTIVEQT
#  include "natportmapper_win_activeqt.h"
# else
#  include "natportmapper_win.h"
# endif
#else
# include "natportmapper_miniupnp.h"
#endif

//------------------- NatPortMapping ----------------------
NatPortMapping::~NatPortMapping()
{

}

//------------------- NatPortMapper ----------------------
NatPortMapper::NatPortMapper(QObject *parent) :
    QObject(parent)
{
    d = new NatPortMapperPrivate(this);
    if (!d->isReady()) {
        connect(d, SIGNAL(initialized()), SLOT(platformReady()));
    } else {
        QTimer::singleShot(0, this, SLOT(platformReady()));
    }
}

void NatPortMapper::platformReady()
{
    emit initialized();
}

bool NatPortMapper::isReady() const
{
    return d->isReady();
}

NatPortMapping *NatPortMapper::map(QAbstractSocket::SocketType socketType, int externalPort,
                                   int internalPort, const QHostAddress &internalAddress,
                                   const QString &description)
{
    return d->add(socketType, externalPort, internalPort, internalAddress, description);
}



