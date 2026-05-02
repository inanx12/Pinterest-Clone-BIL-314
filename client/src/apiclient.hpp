#pragma once
#include <QByteArray>
#include <QObject>
#include <QQueue>
#include <QString>

#include "mediainfo.hpp"

class NetworkClient;
class QFile;

// Protokol komutlarını tipli fonksiyon/sinyal arayüzüne çevirir.
// LoginWindow / MainWindow ham string parse'la uğraşmaz.
//
// Her komut tek tek (FIFO) gönderilir, cevap gelmeden bir sonraki gitmez.
// (Pipelining yok — basit ve doğru kalsın.)
class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(NetworkClient* net, QObject* parent = nullptr);

    // ----- Komutlar -----
    void registerUser(const QString& username, const QString& password);
    void login(const QString& username, const QString& password);
    void logout();

    void listMedia(const QString& sort, int offset = 0, int limit = 20);
    void userMedia(const QString& username, int offset = 0, int limit = 20);

    // Type filePath uzantısından çıkarılır (image|video). visibility: "public"|"private".
    void uploadMedia(const QString& filePath, const QString& visibility);

    void downloadMedia(const QString& mediaId);
    void previewMedia(const QString& mediaId);

    void like(const QString& mediaId);
    void unlike(const QString& mediaId);
    void deleteMedia(const QString& mediaId);

    void changePassword(const QString& oldPassword, const QString& newPassword);

    // ----- Auth durumu -----
    QString token() const    { return token_; }
    QString username() const { return username_; }
    bool isAuthenticated() const { return !token_.isEmpty(); }
    void setSession(const QString& username, const QString& token);
    void clearSession();

signals:
    // Auth
    void registerSucceeded(const QString& token);
    void registerFailed(int code, const QString& message);
    void loginSucceeded(const QString& token);
    void loginFailed(int code, const QString& message);
    void logoutSucceeded();
    void logoutFailed(int code, const QString& message);

    // Feed / profil
    void listSucceeded(const QList<MediaInfo>& items);
    void listFailed(int code, const QString& message);
    void userMediaSucceeded(const QList<MediaInfo>& items);
    void userMediaFailed(int code, const QString& message);

    // Upload
    void uploadProgress(qint64 sent, qint64 total);
    void uploadSucceeded(const QString& mediaId);
    void uploadFailed(int code, const QString& message);

    // Download / preview
    void downloadSucceeded(const QString& mediaId, const QString& type, const QByteArray& data);
    void downloadFailed(const QString& mediaId, int code, const QString& message);
    void previewSucceeded(const QString& mediaId, const QByteArray& data);
    void previewFailed(const QString& mediaId, int code, const QString& message);

    // Like / delete
    void likeSucceeded(const QString& mediaId, int newCount, bool nowLiked);
    void likeFailed(const QString& mediaId, int code, const QString& message);
    void deleteSucceeded(const QString& mediaId);
    void deleteFailed(const QString& mediaId, int code, const QString& message);

    // Şifre
    void changePasswordSucceeded(const QString& newToken);
    void changePasswordFailed(int code, const QString& message);

    // Herhangi bir korumalı komut ERR 1001 alırsa → token geçersiz, login'e dön.
    void tokenInvalidated();

    // Bağlantı / format hatası
    void protocolError(const QString& message);

private slots:
    void onLineReceived(const QString& line);
    void onBinaryReceived(const QByteArray& data);

private:
    enum class Kind {
        Register, Login, Logout,
        List, UserMedia,
        Upload, Download, Preview,
        Like, Unlike, Delete,
        ChangePassword
    };

    struct Pending {
        Kind    kind;
        QString commandLine;       // ham gönderilecek satır
        QString context;            // username (register/login)
        QString mediaId;            // download/preview/like/unlike/delete
        bool    likeRequested = false;  // like vs unlike anlamak için (signal'de nowLiked)
        int     step = 0;
        // upload
        QString uploadFilePath;
        qint64  uploadTotal = 0;
        // download/preview
        QString downloadType;
        qint64  expectedBytes = 0;
    };

    void enqueue(Pending p);
    void pump();
    void completeHead();
    void doUploadChunks();

    static bool parseError(const QString& line, int& code, QString& msg);
    static QString detectType(const QString& filePath);  // image|video|""

    NetworkClient*  net_;
    QString         token_;
    QString         username_;
    QQueue<Pending> queue_;        // baş = in-flight, geri kalanı sırada
    bool            sending_ = false;
};
