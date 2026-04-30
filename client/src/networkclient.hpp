#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QString>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    
    void connectToServer(const QString& host, quint16 port);
    void sendCommand(const QString& cmd);   // satır sonuna \n ekler
    void disconnect();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void textResponseReceived(const QString& line);   // her tam satır

private slots:
    void onReadyRead();
    void onSocketError();

private:
    QTcpSocket* socket_;
    QByteArray  buffer_;   // \n bekleyen kısmi veri
};
