#ifndef SERVERPOOL_H
#define SERVERPOOL_H

#include <QObject>
#include <QVector>
#include <QHostAddress>
#include <QDateTime>
#include <QMutex>

class QTcpSocket;

struct AddressServer {
    QHostAddress host;
    quint16 port;
    bool operator==(const AddressServer& v) const {
        return (host == v.host) && (port == v.port);
    }
};

struct BcServer {
    BcServer(AddressServer address = AddressServer()):
        address(address),
        countConnection(0),
        isActive(true),
        lastTestActive(QDateTime::currentDateTime()) {}
    AddressServer address;
    int countConnection;
    int isActive;
    QDateTime lastTestActive;
    bool operator==(const BcServer& v) const {
        return (address == v.address);
    }
};

class ServerPool: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerPool)
public:
    ServerPool(const QVector<AddressServer> &addressServers, QObject *parent = nullptr);
    QTcpSocket *getSocketServer();
    void decreaseCountConnection(const AddressServer &addressServer);
private:
    QVector<BcServer> m_servers;
    QMutex m_mutexServers;
    void deactivateServer(QVector<BcServer>::iterator serverIt);
    void activateServer(QVector<BcServer>::iterator serverIt);
    QVector<BcServer>::iterator getServerForConnection();
};

#endif // SERVERPOOL_H
