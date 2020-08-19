#include "balancerproxy.h"

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QThreadPool>
#include <QThread>

BalancerProxy::BalancerProxy(const QHostAddress &interfaceListen, quint16 listenPort,
                             const QVector<AddressServer> &addressServers,
                             QObject *parent) : QObject(parent),
    interfaceListen(interfaceListen),
    m_listenPort(listenPort),
    m_server(new QTcpServer(this)),
    m_poolServers(new ServerPool(addressServers, this))
{

}

BalancerProxy::~BalancerProxy()
{
    m_server->close();
    for (auto thread: m_activeThread) {
        thread->quit();
        thread->wait();
    }
}

bool BalancerProxy::startWork()
{
    connect(m_server, &QTcpServer::newConnection, this, &BalancerProxy::newConnect);
    connect(m_server, &QTcpServer::acceptError, this, &BalancerProxy::acceptErrorServer);
    if (!m_server->listen(QHostAddress::Any, m_listenPort)) {
        qDebug() << "Not listen socket";
        return false;
    } else {
        qDebug() << "Start listen port" << m_listenPort;
        return true;
    }
}

void BalancerProxy::newConnect()
{
    while (auto clientSocket = m_server->nextPendingConnection()) {
        auto serverSocket = m_poolServers->getSocketServer();
        if (!serverSocket) {
            clientSocket->close();
            clientSocket->deleteLater();
            return;
        }

        connect(clientSocket, &QTcpSocket::readyRead, [clientSocket, serverSocket] () {
            while (clientSocket->bytesAvailable()) {
                serverSocket->write(clientSocket->readAll());
                serverSocket->flush();
            }
        });
        connect(serverSocket, &QTcpSocket::readyRead, [clientSocket, serverSocket] () {
            while (serverSocket->bytesAvailable()) {
                clientSocket->write(serverSocket->readAll());
                clientSocket->flush();
            }
        });

        connect(clientSocket, &QTcpSocket::disconnected, [this, clientSocket, serverSocket] () {
            m_poolServers->decreaseCountConnection({serverSocket->peerAddress(),
                                                    serverSocket->peerPort()});
            clientSocket->deleteLater();
            serverSocket->close();
        });
        connect(serverSocket, &QTcpSocket::disconnected, [clientSocket, serverSocket] () {
            serverSocket->deleteLater();
            clientSocket->close();
        });

        connect(serverSocket, &QObject::destroyed, [=] () {
            deleteActiveThread(QThread::currentThread());
        });

        connect(clientSocket, &QObject::destroyed, [=] () {
            deleteActiveThread(QThread::currentThread());
        });

        QThread *threadConnection = new QThread(this);
        addActiveThread(threadConnection);
        clientSocket->setParent(nullptr);
        clientSocket->moveToThread(threadConnection);
        serverSocket->moveToThread(threadConnection);
        connect(threadConnection, &QThread::finished, threadConnection, &QThread::deleteLater);
        threadConnection->start();
    }
}

void BalancerProxy::addActiveThread(QThread* thread)
{
    QMutexLocker lock(&m_activeThreadMutex);
    m_activeThread.append(thread);
}

void BalancerProxy::deleteActiveThread(QThread* thread)
{
    QMutexLocker lock(&m_activeThreadMutex);
    if (m_activeThread.removeOne(thread)) {
        thread->quit();
    }
}

void BalancerProxy::acceptErrorServer(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    qDebug() << QStringLiteral("Error TCP server: ") << m_server->errorString();
}
