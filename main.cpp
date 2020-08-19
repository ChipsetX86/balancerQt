#include <QtCore/QCoreApplication>
#include <QCommandLineParser>
#include <QString>
#include <signal.h>
#include "balancerproxy.h"

static const QString SERVER_PARAM = QStringLiteral("server");
static const QString PORT_PARAM = QStringLiteral("port");
static const QString INTERFACE_PARAM = QStringLiteral("interface");

void signal_exit(int) {
    QCoreApplication::quit();
}

QVector<AddressServer> getAddressList(QString text)
{
    QVector<AddressServer> addressList;
    auto list = text.split(",", QString::SkipEmptyParts);
    for (auto item: list) {
        auto server = item.split(":", QString::SkipEmptyParts);
        if (server.count() == 2) {
            addressList.append(AddressServer{QHostAddress(server.at(0)), server.at(1).toUShort()});
        }
    }
    return addressList;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    signal(SIGINT, signal_exit);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({{"p", PORT_PARAM}, "The port on which the balancer server listens.", "port", "5000"});
    parser.addOption({{"s", SERVER_PARAM}, "List of servers to connect format ip:port,ip:port.", "servers"
                      "127.0.0.1:6482"});
    parser.addOption({{"i", INTERFACE_PARAM}, "The interface on which the Balancer server listens.",
                      "interface", "0.0.0.0"});
    parser.parse(app.arguments());
    if (parser.isSet("help")) {
        QTextStream(stdout) << parser.helpText() << endl;
        return 0;
    }
    auto servers = getAddressList(parser.value(SERVER_PARAM));
    QHostAddress networkInterface(parser.value(INTERFACE_PARAM));
    quint16 port(parser.value(PORT_PARAM).toUShort());
    BalancerProxy m_proxy(networkInterface, port, servers, &app);
    if (!m_proxy.startWork()) {
        qDebug() << "Fail start balancer";
        return 1;
    }
    return app.exec();
}
