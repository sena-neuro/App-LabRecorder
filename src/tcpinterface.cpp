#include "tcpinterface.h"
#include <QDebug>

RemoteControlSocket::RemoteControlSocket(uint16_t port) : server() {
	server.listen(QHostAddress::Any, port);
	connect(&server, &QTcpServer::newConnection, this, &RemoteControlSocket::addClient);
}

void RemoteControlSocket::addClient() {
	auto *client = server.nextPendingConnection();
	clients.push_back(client);
	connect(client, &QTcpSocket::readyRead, this, [this, client]() {
		while(client->canReadLine())
			this->handleLine(client->readLine().trimmed(), client);
	});
}

void RemoteControlSocket::handleLine(QString s, QTcpSocket *sock) {
	qInfo() << s;
	const QString command = s.trimmed();
	const QString lowerCommand = command.toLower();
	if (lowerCommand == "start")
		emit start();
	else if (lowerCommand == "stop")
		emit stop();
	else if (lowerCommand == "update")
		emit refresh_streams();
	else if (lowerCommand.startsWith("filename")) {
		emit filename(command);
	} else if (lowerCommand == "select all") {
		emit select_all();
	} else if (lowerCommand == "select none") {
		emit select_none();
	} else if (lowerCommand.startsWith("select ")) {
		emit select_stream(command.mid(QStringLiteral("select ").size()).trimmed());
	}
	sock->write("OK");
	// TODO: support deselecting individual streams.
	// TODO: send acknowledgement
	// TODO: get current state
	//
	// else this->sender()->sender("Whoops");
}
