#include "apiclient.hpp"
#include "networkclient.hpp"
#include "constants.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

ApiClient::ApiClient(NetworkClient* net, QObject* parent)
    : QObject(parent), net_(net) {
    connect(net_, &NetworkClient::lineReceived,   this, &ApiClient::onLineReceived);
    connect(net_, &NetworkClient::binaryReceived, this, &ApiClient::onBinaryReceived);
}

// ----- Session -----

void ApiClient::setSession(const QString& username, const QString& token) {
    username_ = username;
    token_    = token;
}

void ApiClient::clearSession() {
    username_.clear();
    token_.clear();
}

// ----- Yardımcılar -----

bool ApiClient::parseError(const QString& line, int& code, QString& msg) {
    if (!line.startsWith("ERR ")) return false;
    const QString rest = line.mid(4);
    const int sp = rest.indexOf(' ');
    if (sp < 0) { code = rest.toInt(); msg.clear(); }
    else        { code = rest.left(sp).toInt(); msg = rest.mid(sp + 1); }
    return true;
}

QString ApiClient::detectType(const QString& filePath) {
    const QString ext = QFileInfo(filePath).suffix().toLower();
    if (ext == "jpg" || ext == "jpeg" || ext == "png") return "image";
    if (ext == "mp4" || ext == "mov"  || ext == "mkv") return "video";
    return {};
}

void ApiClient::enqueue(Pending p) {
    queue_.enqueue(std::move(p));
    pump();
}

void ApiClient::pump() {
    if (sending_ || queue_.isEmpty()) return;
    sending_ = true;
    Pending& head = queue_.head();
    net_->sendLine(head.commandLine);
}

void ApiClient::completeHead() {
    if (!queue_.isEmpty()) queue_.dequeue();
    sending_ = false;
    pump();
}

// ----- Komutlar -----

void ApiClient::registerUser(const QString& u, const QString& p) {
    Pending pp{};
    pp.kind        = Kind::Register;
    pp.context     = u;
    pp.commandLine = QString("REGISTER %1 %2").arg(u, p);
    enqueue(std::move(pp));
}

void ApiClient::login(const QString& u, const QString& p) {
    Pending pp{};
    pp.kind        = Kind::Login;
    pp.context     = u;
    pp.commandLine = QString("LOGIN %1 %2").arg(u, p);
    enqueue(std::move(pp));
}

void ApiClient::logout() {
    if (token_.isEmpty()) {
        emit logoutSucceeded();
        return;
    }
    if (!net_ || !net_->isConnected()) {
        clearSession();
        emit logoutSucceeded();
        return;
    }
    Pending pp{};
    pp.kind        = Kind::Logout;
    pp.commandLine = QString("LOGOUT %1").arg(token_);
    enqueue(std::move(pp));
}

void ApiClient::listMedia(const QString& sort, int offset, int limit) {
    if (token_.isEmpty()) { emit listFailed(1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit listFailed(0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::List;
    pp.commandLine = QString("LIST %1 %2 %3 %4").arg(token_, sort).arg(offset).arg(limit);
    enqueue(std::move(pp));
}

void ApiClient::userMedia(const QString& username, int offset, int limit) {
    if (token_.isEmpty()) { emit userMediaFailed(1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit userMediaFailed(0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::UserMedia;
    pp.context     = username;
    pp.commandLine = QString("USER_MEDIA %1 %2 %3 %4").arg(token_, username).arg(offset).arg(limit);
    enqueue(std::move(pp));
}

void ApiClient::uploadMedia(const QString& filePath, const QString& visibility) {
    if (token_.isEmpty()) { emit uploadFailed(1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit uploadFailed(0, "Sunucuya bağlı değil."); return; }
    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        emit uploadFailed(2002, "Dosya bulunamadı.");
        return;
    }
    const QString type = detectType(filePath);
    if (type.isEmpty()) { emit uploadFailed(1008, errorMessage(1008)); return; }

    const qint64 size = fi.size();
    const qint64 limit = (type == "video") ? pc::constants::MAX_VIDEO_SIZE : pc::constants::MAX_IMAGE_SIZE;
    if (size > limit) { emit uploadFailed(1007, errorMessage(1007)); return; }

    // Protokol: argümanlar boşlukla ayrılır → filename boşluk/satır sonu içeremez.
    // Otomatik olarak _ ile değiştir.
    QString safeName = fi.fileName();
    safeName.replace(QRegularExpression("[\\s]+"), "_");

    Pending pp{};
    pp.kind            = Kind::Upload;
    pp.uploadFilePath  = filePath;
    pp.uploadTotal     = size;
    pp.commandLine     = QString("UPLOAD %1 %2 %3 %4 %5")
                            .arg(token_, type, visibility, safeName).arg(size);
    enqueue(std::move(pp));
}

void ApiClient::downloadMedia(const QString& mediaId) {
    if (token_.isEmpty()) { emit downloadFailed(mediaId, 1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit downloadFailed(mediaId, 0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::Download;
    pp.mediaId     = mediaId;
    pp.commandLine = QString("DOWNLOAD %1 %2").arg(token_, mediaId);
    enqueue(std::move(pp));
}

void ApiClient::previewMedia(const QString& mediaId) {
    if (token_.isEmpty()) { emit previewFailed(mediaId, 1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit previewFailed(mediaId, 0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::Preview;
    pp.mediaId     = mediaId;
    pp.commandLine = QString("PREVIEW %1 %2").arg(token_, mediaId);
    enqueue(std::move(pp));
}

void ApiClient::like(const QString& mediaId) {
    if (token_.isEmpty()) { emit likeFailed(mediaId, 1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit likeFailed(mediaId, 0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind          = Kind::Like;
    pp.mediaId       = mediaId;
    pp.likeRequested = true;
    pp.commandLine   = QString("LIKE %1 %2").arg(token_, mediaId);
    enqueue(std::move(pp));
}

void ApiClient::unlike(const QString& mediaId) {
    if (token_.isEmpty()) { emit likeFailed(mediaId, 1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit likeFailed(mediaId, 0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind          = Kind::Unlike;
    pp.mediaId       = mediaId;
    pp.likeRequested = false;
    pp.commandLine   = QString("UNLIKE %1 %2").arg(token_, mediaId);
    enqueue(std::move(pp));
}

void ApiClient::deleteMedia(const QString& mediaId) {
    if (token_.isEmpty()) { emit deleteFailed(mediaId, 1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit deleteFailed(mediaId, 0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::Delete;
    pp.mediaId     = mediaId;
    pp.commandLine = QString("DELETE %1 %2").arg(token_, mediaId);
    enqueue(std::move(pp));
}

void ApiClient::changePassword(const QString& oldPwd, const QString& newPwd) {
    if (token_.isEmpty()) { emit changePasswordFailed(1001, errorMessage(1001)); emit tokenInvalidated(); return; }
    if (!net_ || !net_->isConnected()) { emit changePasswordFailed(0, "Sunucuya bağlı değil."); return; }
    Pending pp{};
    pp.kind        = Kind::ChangePassword;
    pp.commandLine = QString("CHANGE_PASSWORD %1 %2 %3").arg(token_, oldPwd, newPwd);
    enqueue(std::move(pp));
}

// ----- UPLOAD chunked send -----

void ApiClient::doUploadChunks() {
    if (queue_.isEmpty()) return;
    Pending& p = queue_.head();
    QFile f(p.uploadFilePath);
    if (!f.open(QIODevice::ReadOnly)) {
        emit uploadFailed(2002, "Dosya okunamadı.");
        completeHead();
        return;
    }

    qint64 sent = 0;
    while (!f.atEnd()) {
        if (!net_->isConnected()) {
            emit uploadFailed(2002, "Bağlantı koptu.");
            completeHead();
            return;
        }
        const QByteArray chunk = f.read(pc::constants::CHUNK_SIZE);
        if (chunk.isEmpty()) break;
        net_->sendRaw(chunk);
        sent += chunk.size();
        emit uploadProgress(sent, p.uploadTotal);
        // Olay döngüsünün socket'i drain etmesine izin ver.
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    f.close();
    // Şimdi step=1, "OK media_id" cevabını bekliyoruz.
}

// ----- Cevap işleme -----

void ApiClient::onLineReceived(const QString& line) {
    if (queue_.isEmpty()) {
        emit protocolError(QString("Beklenmeyen cevap: %1").arg(line));
        return;
    }
    Pending& p = queue_.head();
    int code = 0;
    QString msg;
    const bool isErr = parseError(line, code, msg);

    auto failProtected = [&](auto failSignal) {
        failSignal(code, msg.isEmpty() ? errorMessage(code) : msg);
        if (code == 1001) {
            clearSession();
            emit tokenInvalidated();
        }
    };

    switch (p.kind) {

    // ----- Auth -----
    case Kind::Register:
        if (isErr) {
            emit registerFailed(code, msg.isEmpty() ? errorMessage(code) : msg);
        } else if (line.startsWith("OK ")) {
            const QString token = line.mid(3).trimmed();
            setSession(p.context, token);
            emit registerSucceeded(token);
        } else {
            emit protocolError(QString("REGISTER bozuk cevap: %1").arg(line));
        }
        completeHead();
        break;

    case Kind::Login:
        if (isErr) {
            emit loginFailed(code, msg.isEmpty() ? errorMessage(code) : msg);
        } else if (line.startsWith("OK ")) {
            const QString token = line.mid(3).trimmed();
            setSession(p.context, token);
            emit loginSucceeded(token);
        } else {
            emit protocolError(QString("LOGIN bozuk cevap: %1").arg(line));
        }
        completeHead();
        break;

    case Kind::Logout:
        // Hata bile gelse lokal session temizlenir.
        clearSession();
        if (isErr) emit logoutFailed(code, msg);
        else       emit logoutSucceeded();
        completeHead();
        break;

    // ----- LIST / USER_MEDIA: 2 satır (OK count + json) -----
    case Kind::List:
    case Kind::UserMedia: {
        const bool isList = (p.kind == Kind::List);
        if (p.step == 0) {
            if (isErr) {
                if (isList) failProtected([&](int c, const QString& m){ emit listFailed(c, m); });
                else        failProtected([&](int c, const QString& m){ emit userMediaFailed(c, m); });
                completeHead();
            } else if (line == "OK 0" || line == "OK") {
                if (isList) emit listSucceeded({});
                else        emit userMediaSucceeded({});
                completeHead();
            } else if (line.startsWith("OK ")) {
                p.step = 1;  // sıradaki satır JSON
            } else {
                emit protocolError(QString("LIST bozuk cevap: %1").arg(line));
                completeHead();
            }
        } else {
            QString perr;
            const auto items = MediaInfo::parseJson(line, &perr);
            if (!perr.isEmpty()) {
                if (isList) emit listFailed(2002, "JSON parse: " + perr);
                else        emit userMediaFailed(2002, "JSON parse: " + perr);
            } else {
                if (isList) emit listSucceeded(items);
                else        emit userMediaSucceeded(items);
            }
            completeHead();
        }
        break;
    }

    // ----- UPLOAD: READY → chunked send → OK media_id -----
    case Kind::Upload:
        if (p.step == 0) {
            if (isErr) {
                failProtected([&](int c, const QString& m){ emit uploadFailed(c, m); });
                completeHead();
            } else if (line == "READY") {
                p.step = 1;
                doUploadChunks();
                // doUploadChunks sonrası cevap bekleniyor (step 1).
            } else {
                emit protocolError(QString("UPLOAD bozuk cevap: %1").arg(line));
                completeHead();
            }
        } else {
            if (isErr) {
                failProtected([&](int c, const QString& m){ emit uploadFailed(c, m); });
            } else if (line.startsWith("OK ")) {
                emit uploadSucceeded(line.mid(3).trimmed());
            } else {
                emit protocolError(QString("UPLOAD step2 bozuk: %1").arg(line));
            }
            completeHead();
        }
        break;

    // ----- DOWNLOAD: OK type size + binary -----
    case Kind::Download:
        if (p.step == 0) {
            if (isErr) {
                failProtected([&](int c, const QString& m){ emit downloadFailed(p.mediaId, c, m); });
                completeHead();
            } else if (line.startsWith("OK ")) {
                const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() < 3) {
                    emit protocolError(QString("DOWNLOAD bozuk header: %1").arg(line));
                    completeHead();
                } else {
                    p.downloadType   = parts[1];
                    p.expectedBytes  = parts[2].toLongLong();
                    p.step           = 1;
                    if (p.expectedBytes <= 0) {
                        emit downloadSucceeded(p.mediaId, p.downloadType, {});
                        completeHead();
                    } else {
                        net_->expectBinary(p.expectedBytes);
                    }
                }
            } else {
                emit protocolError(QString("DOWNLOAD bozuk cevap: %1").arg(line));
                completeHead();
            }
        }
        // step 1: binary onBinaryReceived'da işlenir.
        break;

    // ----- PREVIEW: OK size + binary -----
    case Kind::Preview:
        if (p.step == 0) {
            if (isErr) {
                failProtected([&](int c, const QString& m){ emit previewFailed(p.mediaId, c, m); });
                completeHead();
            } else if (line.startsWith("OK ")) {
                p.expectedBytes = line.mid(3).trimmed().toLongLong();
                p.step          = 1;
                if (p.expectedBytes <= 0) {
                    emit previewSucceeded(p.mediaId, {});
                    completeHead();
                } else {
                    net_->expectBinary(p.expectedBytes);
                }
            } else {
                emit protocolError(QString("PREVIEW bozuk cevap: %1").arg(line));
                completeHead();
            }
        }
        break;

    // ----- LIKE / UNLIKE: OK new_count -----
    case Kind::Like:
    case Kind::Unlike:
        if (isErr) {
            failProtected([&](int c, const QString& m){ emit likeFailed(p.mediaId, c, m); });
        } else if (line.startsWith("OK ")) {
            const int newCount = line.mid(3).trimmed().toInt();
            emit likeSucceeded(p.mediaId, newCount, p.likeRequested);
        } else {
            emit protocolError(QString("LIKE bozuk cevap: %1").arg(line));
        }
        completeHead();
        break;

    // ----- DELETE -----
    case Kind::Delete:
        if (isErr) {
            failProtected([&](int c, const QString& m){ emit deleteFailed(p.mediaId, c, m); });
        } else if (line == "OK" || line.startsWith("OK")) {
            emit deleteSucceeded(p.mediaId);
        } else {
            emit protocolError(QString("DELETE bozuk cevap: %1").arg(line));
        }
        completeHead();
        break;

    // ----- CHANGE_PASSWORD -----
    case Kind::ChangePassword:
        if (isErr) {
            failProtected([&](int c, const QString& m){ emit changePasswordFailed(c, m); });
        } else if (line.startsWith("OK ")) {
            const QString newToken = line.mid(3).trimmed();
            token_ = newToken;
            emit changePasswordSucceeded(newToken);
        } else {
            emit protocolError(QString("CHANGE_PASSWORD bozuk cevap: %1").arg(line));
        }
        completeHead();
        break;
    }
}

void ApiClient::onBinaryReceived(const QByteArray& data) {
    if (queue_.isEmpty()) {
        emit protocolError("Bekleyen komut yokken binary geldi.");
        return;
    }
    Pending& p = queue_.head();
    if (p.kind == Kind::Download && p.step == 1) {
        emit downloadSucceeded(p.mediaId, p.downloadType, data);
        completeHead();
    } else if (p.kind == Kind::Preview && p.step == 1) {
        emit previewSucceeded(p.mediaId, data);
        completeHead();
    } else {
        emit protocolError("Beklenmeyen binary veri.");
    }
}
