#ifndef BALANCERPROXY_H
#define BALANCERPROXY_H

#include <QObject>
#include <QMutex>
#include "serverpool.h"

class QTcpServer;
class QThread;

class BalancerProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BalancerProxy)
public:
    explicit BalancerProxy(const QHostAddress &interfaceListen, quint16 listenPort,
                           const QVector<AddressServer> &addressServers, QObject *parent = nullptr);
    ~BalancerProxy();
    bool startWork();
private slots:
    void newConnect();
    void acceptErrorServer(QAbstractSocket::SocketError socketError);
private:
    //void deleteActiveSocket();
    QHostAddress interfaceListen;
    quint16 m_listenPort;
    QTcpServer *m_server;
    ServerPool *m_poolServers;
    QList<QThread*> m_activeThread;
    void addActiveThread(QThread *);
    void deleteActiveThread(QThread*);
    QMutex m_activeThreadMutex;
};

#endif // BALANCERPROXY_H
