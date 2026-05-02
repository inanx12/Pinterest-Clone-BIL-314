#include "mediainfo.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QList<MediaInfo> MediaInfo::parseJson(const QString& json, QString* errorOut) {
    QList<MediaInfo> out;
    QJsonParseError pe;
    const auto doc = QJsonDocument::fromJson(json.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError) {
        if (errorOut) *errorOut = pe.errorString();
        return out;
    }
    if (!doc.isArray()) {
        if (errorOut) *errorOut = "Beklenen JSON array değil.";
        return out;
    }
    const auto arr = doc.array();
    for (const auto& v : arr) {
        if (!v.isObject()) continue;
        const auto o = v.toObject();
        MediaInfo m;
        m.id            = o.value("id").toString();
        m.type          = o.value("type").toString("image");
        m.visibility    = o.value("visibility").toString("public");
        m.owner         = o.value("owner").toString();
        m.filename      = o.value("filename").toString();
        m.size          = static_cast<qint64>(o.value("size").toDouble());
        m.createdAt     = static_cast<qint64>(o.value("created_at").toDouble());
        m.downloadCount = o.value("download_count").toInt();
        m.likeCount     = o.value("like_count").toInt();
        m.likedByMe     = o.value("liked_by_me").toBool();
        out.append(m);
    }
    return out;
}

QString errorMessage(int code) {
    switch (code) {
        case 1001: return "Oturum geçersiz, tekrar giriş yapın.";
        case 1002: return "Bu kullanıcı adı zaten alınmış.";
        case 1003: return "Yanlış kullanıcı adı veya şifre.";
        case 1004: return "Kaynak bulunamadı.";
        case 1005: return "Bu işlem için yetkiniz yok.";
        case 1006: return "Geçersiz parametre.";
        case 1007: return "Dosya boyutu çok büyük.";
        case 1008: return "Desteklenmeyen dosya formatı.";
        case 1009: return "Bu medyaya erişim engellendi (private).";
        case 2001: return "Bozuk komut formatı.";
        case 2002: return "Sunucu hatası.";
        default:   return QString("Bilinmeyen hata (%1)").arg(code);
    }
}
