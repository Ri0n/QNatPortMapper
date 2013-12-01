#ifndef NATPORTMAPPER_H
#define NATPORTMAPPER_H

#include <QObject>
#include <QAbstractSocket>
#include <QHostAddress>

class NatPortMapping : public QObject
{
    Q_OBJECT
public:
    NatPortMapping(QObject *parent = 0) : QObject(parent), _autoUnmap(false) { }
    virtual ~NatPortMapping();

    inline QAbstractSocket::SocketType protocol() const { return _proto; }
    inline quint16 internalPort() const { return _internalPort; }
    inline const QHostAddress &internalAddress() const { return _internalAddress; }
    inline quint16 externalPort() const { return _externalPort; }
    virtual QHostAddress externalAddress() const = 0;
    inline const QString &description() const { return _description; }
    virtual void unmap() = 0;

    /* Controls where we have to unmap on destruct. */
    inline void setAutoUnmap(bool state) { _autoUnmap = state; }

signals:
    void mapped();
    void unmapped();
    void error();

protected:
    NatPortMapping(QAbstractSocket::SocketType socketType, int externalPort, int internalPort,
                           const QHostAddress &internalAddress, const QString &description, QObject *parent = 0) :
        QObject(parent),
        _proto(socketType), _externalPort(externalPort), _internalPort(internalPort),
        _internalAddress(internalAddress), _description(description) {}

    bool _autoUnmap;
    QAbstractSocket::SocketType _proto;
    int _externalPort;
    int _internalPort;
    QHostAddress _internalAddress;
    QString _description;
};

class NatPortMapperPrivate;
class NatPortMapper : public QObject
{
    Q_OBJECT
public:
    explicit NatPortMapper(QObject *parent = 0);
    bool isReady() const;

    NatPortMapping* map(QAbstractSocket::SocketType socketType,
                        int externalPort, int internalPort,
                        const QHostAddress &internalAddress, const QString &description);
signals:
    void initialized();
    
public slots:
private slots:
    void platformReady();
private:
    friend class NatPortMapperPrivate;
    NatPortMapperPrivate *d;
};

#endif // NATPORTMAPPER_H
