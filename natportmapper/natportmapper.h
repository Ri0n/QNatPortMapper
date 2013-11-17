#ifndef NATPORTMAPPER_H
#define NATPORTMAPPER_H

#include <QObject>
#include <QAbstractSocket>

class NatPortMapping : public QObject
{
    Q_OBJECT
public:
    NatPortMapping(QObject *parent = 0) : QObject(parent), _autoUnmap(false) { }
    virtual quint16 internalPort() const = 0;
    virtual QHostAddress internalAddress() const = 0;
    virtual quint16 externalPort() const = 0;
    virtual QHostAddress externalAddress() const = 0;
    virtual QString description() const = 0;
    virtual bool unmap() = 0;

    /* Controls where we have to unmap on destruct. */
    inline void setAutoUnmap(bool state) { _autoUnmap = state; }

signals:
    void mapped();
    void unmapped();
    void error();
protected:
    virtual ~NatPortMapping();
protected:
    bool _autoUnmap;
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
