#include "networkclient.hpp"

#include <QTimer>
#include <algorithm>

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent),
      socket_(new QTcpSocket(this)),
      reconnectTimer_(new QTimer(this)) {
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &NetworkClient::tryReconnect);

    connect(socket_, &QTcpSocket::connected,    this, &NetworkClient::connected);
    connect(socket_, &QTcpSocket::disconnected, this, &NetworkClient::onSocketDisconnected);
    connect(socket_, &QTcpSocket::readyRead,    this, &NetworkClient::onReadyRead);
    connect(socket_, &QTcpSocket::errorOccurred, this, &NetworkClient::onSocketError);
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    lastHost_ = host;
    lastPort_ = port;
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        socket_->abort();
    }
    buffer_.clear();
    binaryAccum_.clear();
    binaryRemaining_ = 0;
    socket_->connectToHost(host, port);
}

void NetworkClient::disconnectFromServer() {
    autoReconnect_ = false;  // kullanıcı isteyerek kestiyse otomatik dönme
    reconnectTimer_->stop();
    if (socket_->state() == QAbstractSocket::UnconnectedState) return;
    socket_->disconnectFromHost();
}

bool NetworkClient::isConnected() const {
    return socket_->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::sendLine(const QString& line) {
    QByteArray payload = line.toUtf8();
    payload.append('\n');
    socket_->write(payload);
}

void NetworkClient::sendRaw(const QByteArray& data) {
    socket_->write(data);
}

void NetworkClient::expectBinary(qint64 bytes) {
    binaryRemaining_ = bytes;
    binaryAccum_.clear();
    binaryAccum_.reserve(static_cast<int>(bytes));
    // Buffer'da zaten okunmuş veri olabilir.
    processBuffer();
}

void NetworkClient::setAutoReconnect(bool on, int delayMs) {
    autoReconnect_ = on;
    reconnectDelayMs_ = delayMs;
}

void NetworkClient::onReadyRead() {
    buffer_.append(socket_->readAll());
    processBuffer();
}

void NetworkClient::processBuffer() {
    if (binaryRemaining_ > 0 && !buffer_.isEmpty()) {
        const qint64 take = std::min<qint64>(binaryRemaining_, buffer_.size());
        binaryAccum_.append(buffer_.left(static_cast<int>(take)));
        buffer_.remove(0, static_cast<int>(take));
        binaryRemaining_ -= take;
        if (binaryRemaining_ == 0) {
            QByteArray completed = binaryAccum_;
            binaryAccum_.clear();
            emit binaryReceived(completed);
        }
    }

    while (binaryRemaining_ == 0) {
        const int nl = buffer_.indexOf('\n');
        if (nl < 0) break;
        const QByteArray line = buffer_.left(nl);
        buffer_.remove(0, nl + 1);
        emit lineReceived(QString::fromUtf8(line));
    }
}

void NetworkClient::onSocketError(QAbstractSocket::SocketError) {
    emit errorOccurred(socket_->errorString());
}

void NetworkClient::onSocketDisconnected() {
    // Yarım kalan binary state'i temizle ki eski veri yeni bağlantıya sızmasın.
    buffer_.clear();
    binaryAccum_.clear();
    binaryRemaining_ = 0;

    emit disconnected();

    if (autoReconnect_ && !lastHost_.isEmpty() && lastPort_ != 0
        && !reconnectTimer_->isActive()) {
        reconnectTimer_->start(reconnectDelayMs_);
    }
}

void NetworkClient::tryReconnect() {
    if (!autoReconnect_) return;
    if (socket_->state() != QAbstractSocket::UnconnectedState) return;
    socket_->connectToHost(lastHost_, lastPort_);
}
