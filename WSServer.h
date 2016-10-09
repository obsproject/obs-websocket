#ifndef WSSERVER_H
#define WSSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject
{
	Q_OBJECT

	public:
		explicit WSServer(quint16 port, QObject *parent = Q_NULLPTR);
		virtual ~WSServer();
		void broadcast(QString message);

	private Q_SLOTS:
		void onNewConnection();
		void processTextMessage(QString textMessage);
		void socketDisconnected();

	private:
		QWebSocketServer *_wsServer;
		QList<QWebSocket *> _clients;
};

#endif // WSSERVER_H