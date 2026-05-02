#pragma once
#include <QHash>
#include <QMainWindow>
#include <QPointer>
#include <QString>

#include "mediainfo.hpp"

class ApiClient;
class MediaCard;

class QButtonGroup;
class QComboBox;
class QGridLayout;
class QLabel;
class QProgressDialog;
class QPushButton;
class QStackedWidget;
class QToolButton;
class QWidget;

// Ana pencere — toolbar (Yükle/Sırala/Çıkış) + sol Feed/Profil + sağ tab paneli.
// Tüm veriler ApiClient üzerinden server'dan gelir; yerel sahte veri yok.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(ApiClient* api, QWidget* parent = nullptr);

    // LoginWindow başarılı auth sonrası çağırır → ilk feed yüklemesini tetikler.
    void onAuthenticated();

private slots:
    // Toolbar
    void onUploadClicked();
    void onSortChanged(int idx);
    void onLogoutClicked();
    void onLogoutSucceeded();
    void onLogoutFailed(int code, const QString& msg);
    void onRefreshClicked();
    void onChangePasswordClicked();

    // Sağ tab
    void onShowFeed();
    void onShowProfile();

    // ApiClient — feed
    void onListSucceeded(const QList<MediaInfo>& items);
    void onListFailed(int code, const QString& msg);
    void onUserMediaSucceeded(const QList<MediaInfo>& items);
    void onUserMediaFailed(int code, const QString& msg);

    // ApiClient — preview / download
    void onPreviewSucceeded(const QString& mediaId, const QByteArray& data);
    void onPreviewFailed(const QString& mediaId, int code, const QString& msg);
    void onDownloadSucceeded(const QString& mediaId, const QString& type, const QByteArray& data);
    void onDownloadFailed(const QString& mediaId, int code, const QString& msg);

    // ApiClient — upload
    void onUploadProgress(qint64 sent, qint64 total);
    void onUploadSucceeded(const QString& mediaId);
    void onUploadFailed(int code, const QString& msg);

    // ApiClient — like / delete / change password
    void onLikeSucceeded(const QString& mediaId, int newCount, bool nowLiked);
    void onLikeFailed(const QString& mediaId, int code, const QString& msg);
    void onDeleteSucceeded(const QString& mediaId);
    void onDeleteFailed(const QString& mediaId, int code, const QString& msg);
    void onChangePasswordSucceeded(const QString& newToken);
    void onChangePasswordFailed(int code, const QString& msg);

    // Card sinyalleri
    void onCardOpenRequested(const QString& mediaId);
    void onCardLikeToggled(const QString& mediaId, bool nowLiked);
    void onCardDeleteRequested(const QString& mediaId);

private:
    void buildUi();
    QWidget* buildFeedPage();
    QWidget* buildProfilePage();
    QWidget* buildSideTabs();
    void     applyStyle();

    void clearGrid(QGridLayout* grid, int& row, int& col);
    void renderItems(QGridLayout* grid, int& row, int& col, const QList<MediaInfo>& items, bool registerCards);
    MediaCard* makeCard(const MediaInfo& m);
    void requestThumbnailIfImage(MediaCard* card, const QString& id, const QString& type);

    void refreshFeed();
    void refreshProfile();
    void refreshGreeting();

    QString currentSortKey() const;
    QString fileExtensionFor(const QString& mediaId) const;
    QString filenameOf(const QString& mediaId) const;
    QString ownerOf(const QString& mediaId) const;
    bool    isVideo(const QString& mediaId) const;

    void showError(const QString& title, int code, const QString& msg);

    ApiClient* api_;

    QStackedWidget* pages_         = nullptr;
    QComboBox*      sortCombo_     = nullptr;
    QToolButton*    mediaTabBtn_   = nullptr;
    QToolButton*    profileTabBtn_ = nullptr;
    QButtonGroup*   tabGroup_      = nullptr;
    QPushButton*    uploadBtn_     = nullptr;
    QPushButton*    refreshBtn_    = nullptr;
    QPushButton*    logoutBtn_     = nullptr;

    // Feed sayfası
    QGridLayout* feedGrid_     = nullptr;
    QLabel*      feedEmptyLbl_ = nullptr;
    int          feedRow_      = 0;
    int          feedCol_      = 0;

    // Profil sayfası
    QLabel*       profUserLbl_   = nullptr;
    QLabel*       profStatLbl_   = nullptr;
    QGridLayout*  profGrid_      = nullptr;
    QLabel*       profEmptyLbl_  = nullptr;
    QPushButton*  changePwdBtn_  = nullptr;
    int           profRow_       = 0;
    int           profCol_       = 0;

    // Yardımcı durum
    QHash<QString, MediaInfo>            mediaIndex_;     // id → bilgi (filename/type/owner)
    QHash<QString, QPixmap>              thumbCache_;     // id → thumbnail pixmap
    QHash<QString, QList<QPointer<MediaCard>>> waitingThumbs_;  // id → o id'yi bekleyen kartlar
    QProgressDialog*                     uploadProgress_ = nullptr;
    QString                              uploadingFileName_;
};
