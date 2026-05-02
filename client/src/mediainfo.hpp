#pragma once
#include <QList>
#include <QString>

// LIST / USER_MEDIA cevaplarındaki tek medya kaydı.
struct MediaInfo {
    QString id;
    QString type;          // "image" | "video"
    QString visibility;    // "public" | "private"
    QString owner;
    QString filename;
    qint64  size           = 0;
    qint64  createdAt      = 0;
    int     downloadCount  = 0;
    int     likeCount      = 0;
    bool    likedByMe      = false;

    // Server'dan gelen JSON satırını parse eder.
    // Hata olursa errorOut doldurulur, dönüş boş liste olur.
    static QList<MediaInfo> parseJson(const QString& json, QString* errorOut = nullptr);
};

// Protokol hata kodu → kullanıcıya gösterilecek Türkçe mesaj.
QString errorMessage(int code);
