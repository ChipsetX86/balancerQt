#include "serverpool.h"

#include <QTcpSocket>
#include <QDebug>
#include <QMutexLocker>

static const int TIME_WAIT_CONNECT_MSECONDS = 200;
static const int TIME_DEACTIVATION_SERVER_SECONDS = 20;

ServerPool::ServerPool(const QVector<AddressServer> &addressServers, QObject *parent):
    QObject(parent)
{
    for (const auto &server: addressServers) {
        if (m_servers.indexOf(server) == -1) {
            m_servers.append(server);
        }
    }
}

QTcpSocket *ServerPool::getSocketServer()
{
    if (!m_servers.count()) {
        return nullptr;
    }

    QVector<BcServer>::iterator minConnectionServer;
    while ((minConnectionServer = getServerForConnection()) != m_servers.end()) {
        auto socket = new QTcpSocket();
        socket->connectToHost(minConnectionServer->address.host, minConnectionServer->address.port);
        if (socket->waitForConnected(TIME_WAIT_CONNECT_MSECONDS)) {
            qDebug() << QStringLiteral("Connect to server %1:%2").
                        arg(minConnectionServer->address.host.toString(),
                            QString::number(minConnectionServer->address.port));
            minConnectionServer->countConnection++;
            activateServer(minConnectionServer);
            return socket;
        } else {
            deactivateServer(minConnectionServer);
            qDebug() << QStringLiteral("Not connect to server %1:%2").
                        arg(minConnectionServer->address.host.toString(),
                            QString::number(minConnectionServer->address.port));
            delete socket;
        }
    }

    qDebug() << "Not found server";
    return nullptr;
}

void ServerPool::decreaseCountConnection(const AddressServer &addressServer)
{
    QMutexLocker lock(&m_mutexServers);
    QVector<BcServer>::iterator current = m_servers.begin();
    while (current != m_servers.end()) {
        if (current->address == addressServer) {
            if (current->countConnection > 0)
                current->countConnection--;
        }
        ++current;
    }
}

void ServerPool::deactivateServer(QVector<BcServer>::iterator serverIt)
{
    serverIt->isActive = false;
    serverIt->lastTestActive = QDateTime::currentDateTime();
}

void ServerPool::activateServer(QVector<BcServer>::iterator serverIt)
{
    serverIt->isActive = true;
    serverIt->lastTestActive = QDateTime::currentDateTime();
}

QVector<BcServer>::iterator ServerPool::getServerForConnection()
{
    bool flagFirst = true;
    QVector<BcServer>::iterator minConnectionServer = m_servers.end();
    QVector<BcServer>::iterator current = m_servers.begin();
    while (current != m_servers.end()) {
        if (current->isActive ||
                current->lastTestActive.secsTo(QDateTime::currentDateTime()) >
                TIME_DEACTIVATION_SERVER_SECONDS) {
            if (flagFirst || (minConnectionServer->countConnection > current->countConnection)) {
                minConnectionServer = current;
                flagFirst = false;
            }
        }
        ++current;
    }
    return minConnectionServer;
}
