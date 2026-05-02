#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>

class QTimer;

// Saf transport katmanı: TCP socket + line-based / binary okuma + auto-reconnect.
// Protokol bilgisi yok — onu ApiClient yapar.
class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    void sendLine(const QString& line);          // sona \n ekler
    void sendRaw(const QByteArray& data);        // UPLOAD payload için
    void expectBinary(qint64 bytes);             // sonraki N byte binary olarak toplanır

    // Bağlantı koparsa belirli aralıkla yeniden dener.
    void setAutoReconnect(bool on, int delayMs = 2000);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void lineReceived(const QString& line);
    void binaryReceived(const QByteArray& data);

private slots:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError);
    void onSocketDisconnected();
    void tryReconnect();

private:
    void processBuffer();

    QTcpSocket* socket_;
    QByteArray  buffer_;
    qint64      binaryRemaining_ = 0;
    QByteArray  binaryAccum_;

    // Auto-reconnect
    QString     lastHost_;
    quint16     lastPort_         = 0;
    bool        autoReconnect_    = false;
    int         reconnectDelayMs_ = 2000;
    QTimer*     reconnectTimer_;
};
