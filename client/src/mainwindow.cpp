#include "mainwindow.hpp"
#include "apiclient.hpp"
#include "imageviewer.hpp"
#include "mediacard.hpp"
#include "videoviewer.hpp"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kGridCols = 4;

const char* kStyleSheet = R"(
QMainWindow, QWidget#feedPage, QWidget#profilePage {
    background: #f4f4f4;
}
QToolBar {
    background: #ececec;
    border: 0;
    border-bottom: 1px solid #d0d0d0;
    spacing: 8px;
    padding: 6px;
}
QToolBar QPushButton {
    background: #ffffff;
    border: 1px solid #c8c8c8;
    border-radius: 4px;
    padding: 5px 14px;
    color: #2b2b2b;
}
QToolBar QPushButton:hover { background: #e8e8e8; }
QToolBar QPushButton:pressed { background: #d8d8d8; }
QToolBar QLabel { color: #555; padding: 0 4px; }
QComboBox {
    background: #ffffff;
    border: 1px solid #c8c8c8;
    border-radius: 4px;
    padding: 4px 8px;
    color: #2b2b2b;
    min-width: 170px;
}
QComboBox QAbstractItemView {
    background: #ffffff;
    border: 1px solid #c8c8c8;
    selection-background-color: #d8d8d8;
    selection-color: #2b2b2b;
}
QStatusBar { background: #ececec; color: #555; }
QFrame#mediaCard {
    background: #ffffff;
    border: 1px solid #d6d6d6;
    border-radius: 6px;
}
QFrame#mediaCard[hover="true"] {
    border: 1px solid #888888;
    background: #fafafa;
}
QLabel#cardFilename { color: #2b2b2b; font-weight: 600; }
QLabel#cardOwner    { color: #777777; font-size: 11px; }
QLabel#typeBadge {
    background: rgba(0, 0, 0, 0.55);
    color: #ffffff;
    border-radius: 14px;
    font-weight: bold;
}
QLabel#visibilityBadge {
    background: rgba(0, 0, 0, 0.55);
    color: #ffffff;
    border-radius: 14px;
    font-size: 14px;
}
QPushButton#likeBtn {
    color: #555;
    padding: 2px 6px;
    border: 0;
}
QPushButton#likeBtn[liked="true"] { color: #444; font-weight: bold; }
QPushButton#likeBtn:hover { color: #222; }
QPushButton#deleteBtn {
    color: #777;
    padding: 2px 6px;
    border: 0;
}
QPushButton#deleteBtn:hover { color: #2b2b2b; text-decoration: underline; }
QWidget#sideTabs {
    background: #e6e6e6;
    border-left: 1px solid #d0d0d0;
}
QToolButton#sideTabBtn {
    background: #e6e6e6;
    border: 0;
    border-bottom: 1px solid #d0d0d0;
    padding: 18px 8px;
    color: #555;
    font-weight: 600;
}
QToolButton#sideTabBtn:hover { background: #dddddd; color: #2b2b2b; }
QToolButton#sideTabBtn:checked {
    background: #ffffff;
    color: #2b2b2b;
    border-left: 3px solid #555;
}
QLabel#emptyHint { color: #999; font-size: 14px; }
QLabel#profUser { color: #2b2b2b; font-size: 18px; font-weight: 600; }
QLabel#profStat { color: #666; }
QLabel#sectionTitle { color: #444; font-size: 14px; font-weight: 600; }
QPushButton#changePwdBtn {
    background: #ffffff;
    border: 1px solid #c8c8c8;
    border-radius: 4px;
    padding: 5px 14px;
    color: #2b2b2b;
}
QPushButton#changePwdBtn:hover { background: #e8e8e8; }
)";
}  // namespace

MainWindow::MainWindow(ApiClient* api, QWidget* parent)
    : QMainWindow(parent), api_(api) {
    setWindowTitle("Pinterest-Clone");
    resize(1180, 780);
    buildUi();
    applyStyle();

    // ApiClient sinyalleri
    connect(api_, &ApiClient::logoutSucceeded,    this, &MainWindow::onLogoutSucceeded);
    connect(api_, &ApiClient::logoutFailed,       this, &MainWindow::onLogoutFailed);
    connect(api_, &ApiClient::listSucceeded,      this, &MainWindow::onListSucceeded);
    connect(api_, &ApiClient::listFailed,         this, &MainWindow::onListFailed);
    connect(api_, &ApiClient::userMediaSucceeded, this, &MainWindow::onUserMediaSucceeded);
    connect(api_, &ApiClient::userMediaFailed,    this, &MainWindow::onUserMediaFailed);
    connect(api_, &ApiClient::previewSucceeded,   this, &MainWindow::onPreviewSucceeded);
    connect(api_, &ApiClient::previewFailed,      this, &MainWindow::onPreviewFailed);
    connect(api_, &ApiClient::downloadSucceeded,  this, &MainWindow::onDownloadSucceeded);
    connect(api_, &ApiClient::downloadFailed,     this, &MainWindow::onDownloadFailed);
    connect(api_, &ApiClient::uploadProgress,     this, &MainWindow::onUploadProgress);
    connect(api_, &ApiClient::uploadSucceeded,    this, &MainWindow::onUploadSucceeded);
    connect(api_, &ApiClient::uploadFailed,       this, &MainWindow::onUploadFailed);
    connect(api_, &ApiClient::likeSucceeded,      this, &MainWindow::onLikeSucceeded);
    connect(api_, &ApiClient::likeFailed,         this, &MainWindow::onLikeFailed);
    connect(api_, &ApiClient::deleteSucceeded,    this, &MainWindow::onDeleteSucceeded);
    connect(api_, &ApiClient::deleteFailed,       this, &MainWindow::onDeleteFailed);
    connect(api_, &ApiClient::changePasswordSucceeded, this, &MainWindow::onChangePasswordSucceeded);
    connect(api_, &ApiClient::changePasswordFailed,    this, &MainWindow::onChangePasswordFailed);

    // Klavye kısayolları
    new QShortcut(QKeySequence("Ctrl+U"), this, this, &MainWindow::onUploadClicked);
    new QShortcut(QKeySequence("F5"),     this, this, &MainWindow::onRefreshClicked);
    new QShortcut(QKeySequence("Ctrl+L"), this, this, &MainWindow::onLogoutClicked);

    refreshGreeting();
}

void MainWindow::buildUi() {
    auto* tb = addToolBar("Ana");
    tb->setMovable(false);
    tb->setFloatable(false);

    uploadBtn_ = new QPushButton("⤴  Yükle", tb);
    uploadBtn_->setCursor(Qt::PointingHandCursor);
    tb->addWidget(uploadBtn_);

    auto* sep1 = new QWidget(tb);
    sep1->setFixedWidth(8);
    tb->addWidget(sep1);

    refreshBtn_ = new QPushButton("⟳  Yenile", tb);
    refreshBtn_->setCursor(Qt::PointingHandCursor);
    tb->addWidget(refreshBtn_);

    auto* sep2 = new QWidget(tb);
    sep2->setFixedWidth(12);
    tb->addWidget(sep2);

    auto* sortLbl = new QLabel("Sırala:", tb);
    tb->addWidget(sortLbl);

    sortCombo_ = new QComboBox(tb);
    sortCombo_->addItem("En yeni",          "newest");
    sortCombo_->addItem("En çok beğenilen", "most_liked");
    sortCombo_->addItem("En çok indirilen", "most_downloaded");
    tb->addWidget(sortCombo_);

    auto* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    logoutBtn_ = new QPushButton("⏻  Çıkış", tb);
    logoutBtn_->setCursor(Qt::PointingHandCursor);
    tb->addWidget(logoutBtn_);

    connect(uploadBtn_,  &QPushButton::clicked, this, &MainWindow::onUploadClicked);
    connect(refreshBtn_, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(logoutBtn_,  &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(sortCombo_,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortChanged);

    // Central
    auto* central = new QWidget(this);
    auto* h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    pages_ = new QStackedWidget(central);
    pages_->addWidget(buildFeedPage());
    pages_->addWidget(buildProfilePage());

    h->addWidget(pages_, 1);
    h->addWidget(buildSideTabs(), 0);
    setCentralWidget(central);

    statusBar()->showMessage("Hazır.");
}

QWidget* MainWindow::buildFeedPage() {
    auto* page = new QWidget;
    page->setObjectName("feedPage");
    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* inner = new QWidget;
    inner->setObjectName("feedPage");
    auto* innerLay = new QVBoxLayout(inner);
    innerLay->setContentsMargins(20, 20, 20, 20);
    innerLay->setSpacing(12);

    feedGrid_ = new QGridLayout;
    feedGrid_->setSpacing(16);
    feedGrid_->setContentsMargins(0, 0, 0, 0);

    feedEmptyLbl_ = new QLabel("Henüz medya yok. \"Yükle\" butonuyla ilk gönderini ekle.");
    feedEmptyLbl_->setObjectName("emptyHint");
    feedEmptyLbl_->setAlignment(Qt::AlignCenter);
    feedEmptyLbl_->hide();

    innerLay->addWidget(feedEmptyLbl_, 0, Qt::AlignTop);
    innerLay->addLayout(feedGrid_);
    innerLay->addStretch(1);
    scroll->setWidget(inner);

    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
    return page;
}

QWidget* MainWindow::buildProfilePage() {
    auto* page = new QWidget;
    page->setObjectName("profilePage");
    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* inner = new QWidget;
    inner->setObjectName("profilePage");
    auto* innerLay = new QVBoxLayout(inner);
    innerLay->setContentsMargins(24, 24, 24, 24);
    innerLay->setSpacing(10);

    auto* hdrTitle = new QLabel("Ana Medya");
    hdrTitle->setObjectName("sectionTitle");

    profUserLbl_ = new QLabel("kullanıcı adı: —");
    profUserLbl_->setObjectName("profUser");

    profStatLbl_ = new QLabel("Yüklediği medya: 0");
    profStatLbl_->setObjectName("profStat");

    changePwdBtn_ = new QPushButton("Şifre değiştir");
    changePwdBtn_->setObjectName("changePwdBtn");
    changePwdBtn_->setCursor(Qt::PointingHandCursor);

    auto* userRow = new QHBoxLayout;
    userRow->addWidget(profUserLbl_);
    userRow->addStretch();
    userRow->addWidget(changePwdBtn_);

    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #d0d0d0;");

    auto* uploadedTitle = new QLabel("Yüklenen fotoğraflar");
    uploadedTitle->setObjectName("sectionTitle");

    profGrid_ = new QGridLayout;
    profGrid_->setSpacing(16);
    profGrid_->setContentsMargins(0, 0, 0, 0);

    profEmptyLbl_ = new QLabel("Henüz hiçbir şey yüklemedin.");
    profEmptyLbl_->setObjectName("emptyHint");
    profEmptyLbl_->hide();

    auto* infoTitle = new QLabel("Bilgiler");
    infoTitle->setObjectName("sectionTitle");

    auto* infoLbl = new QLabel(
        "• Public medyalar herkes tarafından görülebilir.\n"
        "• Private medyalar sadece sana görünür.\n"
        "• Kendi medyalarında sil butonu aktiftir.");
    infoLbl->setStyleSheet("color: #666;");
    infoLbl->setWordWrap(true);

    innerLay->addWidget(hdrTitle);
    innerLay->addLayout(userRow);
    innerLay->addWidget(profStatLbl_);
    innerLay->addSpacing(8);
    innerLay->addWidget(sep);
    innerLay->addSpacing(8);
    innerLay->addWidget(uploadedTitle);
    innerLay->addWidget(profEmptyLbl_);
    innerLay->addLayout(profGrid_);
    innerLay->addSpacing(20);
    innerLay->addWidget(infoTitle);
    innerLay->addWidget(infoLbl);
    innerLay->addStretch(1);

    scroll->setWidget(inner);
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    connect(changePwdBtn_, &QPushButton::clicked, this, &MainWindow::onChangePasswordClicked);
    return page;
}

QWidget* MainWindow::buildSideTabs() {
    auto* w = new QWidget;
    w->setObjectName("sideTabs");
    w->setFixedWidth(70);

    mediaTabBtn_ = new QToolButton(w);
    mediaTabBtn_->setObjectName("sideTabBtn");
    mediaTabBtn_->setText("Medya");
    mediaTabBtn_->setCheckable(true);
    mediaTabBtn_->setChecked(true);
    mediaTabBtn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mediaTabBtn_->setCursor(Qt::PointingHandCursor);

    profileTabBtn_ = new QToolButton(w);
    profileTabBtn_->setObjectName("sideTabBtn");
    profileTabBtn_->setText("Profil");
    profileTabBtn_->setCheckable(true);
    profileTabBtn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    profileTabBtn_->setCursor(Qt::PointingHandCursor);

    tabGroup_ = new QButtonGroup(this);
    tabGroup_->setExclusive(true);
    tabGroup_->addButton(mediaTabBtn_, 0);
    tabGroup_->addButton(profileTabBtn_, 1);

    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(mediaTabBtn_);
    lay->addWidget(profileTabBtn_);
    lay->addStretch(1);

    connect(mediaTabBtn_,   &QToolButton::clicked, this, &MainWindow::onShowFeed);
    connect(profileTabBtn_, &QToolButton::clicked, this, &MainWindow::onShowProfile);
    return w;
}

void MainWindow::applyStyle() {
    setStyleSheet(kStyleSheet);
}

// ----- Auth sonrası giriş -----

void MainWindow::onAuthenticated() {
    refreshGreeting();
    refreshFeed();
}

void MainWindow::refreshGreeting() {
    const QString u = api_->username();
    profUserLbl_->setText(u.isEmpty() ? "kullanıcı adı: —"
                                      : QString("kullanıcı adı: %1").arg(u));
    setWindowTitle(u.isEmpty() ? "Pinterest-Clone"
                               : QString("Pinterest-Clone — %1").arg(u));
}

QString MainWindow::currentSortKey() const {
    return sortCombo_ ? sortCombo_->currentData().toString() : QString("newest");
}

void MainWindow::refreshFeed() {
    if (!api_->isAuthenticated()) return;
    statusBar()->showMessage("Feed yükleniyor…");
    api_->listMedia(currentSortKey(), 0, 20);
}

void MainWindow::refreshProfile() {
    if (!api_->isAuthenticated()) return;
    statusBar()->showMessage("Profil yükleniyor…");
    api_->userMedia(api_->username(), 0, 20);
}

// ----- Grid yardımcıları -----

void MainWindow::clearGrid(QGridLayout* grid, int& row, int& col) {
    while (QLayoutItem* item = grid->takeAt(0)) {
        if (auto* w = item->widget()) w->deleteLater();
        delete item;
    }
    row = 0;
    col = 0;
}

MediaCard* MainWindow::makeCard(const MediaInfo& m) {
    auto* c = new MediaCard;
    c->setMediaId(m.id);
    c->setFilename(m.filename);
    c->setOwner(m.owner);
    c->setType(m.type == "video" ? MediaCard::Type::Video : MediaCard::Type::Image);
    c->setVisibility(m.visibility == "private" ? MediaCard::Visibility::Private
                                                : MediaCard::Visibility::Public);
    c->setLikeCount(m.likeCount);
    c->setLikedByMe(m.likedByMe);
    c->setOwnedByMe(!api_->username().isEmpty() && m.owner == api_->username());

    // Cache'te thumbnail varsa direkt ata
    if (thumbCache_.contains(m.id)) {
        c->setThumbnail(thumbCache_.value(m.id));
    }

    connect(c, &MediaCard::openRequested,   this, &MainWindow::onCardOpenRequested);
    connect(c, &MediaCard::likeToggled,     this, &MainWindow::onCardLikeToggled);
    connect(c, &MediaCard::deleteRequested, this, &MainWindow::onCardDeleteRequested);
    return c;
}

void MainWindow::renderItems(QGridLayout* grid, int& row, int& col,
                             const QList<MediaInfo>& items, bool registerCards) {
    clearGrid(grid, row, col);
    for (const auto& m : items) {
        mediaIndex_.insert(m.id, m);
        auto* card = makeCard(m);
        grid->addWidget(card, row, col);
        if (++col >= kGridCols) { col = 0; ++row; }
        if (registerCards && m.type == "image") {
            requestThumbnailIfImage(card, m.id, m.type);
        }
    }
}

void MainWindow::requestThumbnailIfImage(MediaCard* card, const QString& id, const QString& type) {
    if (type != "image") return;
    if (thumbCache_.contains(id)) {
        card->setThumbnail(thumbCache_.value(id));
        return;
    }
    auto& list = waitingThumbs_[id];
    const bool firstRequest = list.isEmpty();
    list.append(QPointer<MediaCard>(card));
    if (firstRequest) api_->previewMedia(id);
}

// ----- Toolbar slotları -----

void MainWindow::onUploadClicked() {
    if (!api_->isAuthenticated()) {
        QMessageBox::warning(this, "Yükle", "Önce giriş yapmalısın.");
        return;
    }
    const QString path = QFileDialog::getOpenFileName(
        this, "Medya seç", QString(),
        "Medya (*.jpg *.jpeg *.png *.mp4 *.mov *.mkv);;Tüm dosyalar (*)");
    if (path.isEmpty()) return;

    QFileInfo fi(path);
    const QString ext = fi.suffix().toLower();
    const bool isImage = (ext == "jpg" || ext == "jpeg" || ext == "png");
    const bool isVideo = (ext == "mp4" || ext == "mov"  || ext == "mkv");
    if (!isImage && !isVideo) {
        QMessageBox::warning(this, "Yükle",
            "Desteklenmeyen format. Resim: jpg/jpeg/png — Video: mp4/mov/mkv.");
        return;
    }
    // Boyut pre-check (server'a gitmeden uyar)
    const qint64 sz = fi.size();
    const qint64 limit = isVideo ? (100LL * 1024 * 1024) : (20LL * 1024 * 1024);
    if (sz > limit) {
        QMessageBox::warning(this, "Yükle",
            QString("Dosya çok büyük (%1 MB). Limit: %2 MB.")
                .arg(sz / (1024 * 1024))
                .arg(limit / (1024 * 1024)));
        return;
    }

    // Visibility dialog
    QDialog dlg(this);
    dlg.setWindowTitle("Yükleme ayarları");
    auto* form = new QFormLayout(&dlg);
    auto* pubRb = new QRadioButton("Public — herkes görsün");
    auto* prvRb = new QRadioButton("Private — sadece ben göreyim");
    pubRb->setChecked(true);
    form->addRow(new QLabel(QString("<b>Dosya:</b> %1").arg(fi.fileName())));
    form->addRow(new QLabel(QString("<b>Boyut:</b> %1 KB").arg(sz / 1024)));
    form->addRow(new QLabel(QString("<b>Tip:</b> %1").arg(isVideo ? "video" : "image")));
    form->addRow(pubRb);
    form->addRow(prvRb);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText("Yükle");
    bb->button(QDialogButtonBox::Cancel)->setText("Vazgeç");
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(bb);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString visibility = pubRb->isChecked() ? "public" : "private";

    // Progress dialog
    if (uploadProgress_) { uploadProgress_->deleteLater(); uploadProgress_ = nullptr; }
    uploadingFileName_ = fi.fileName();
    uploadProgress_ = new QProgressDialog(
        QString("\"%1\" yükleniyor…").arg(fi.fileName()),
        QString(),  // Cancel butonu yok (chunked send sync, iptal karmaşık)
        0, 100, this);
    uploadProgress_->setWindowModality(Qt::WindowModal);
    uploadProgress_->setMinimumDuration(0);
    uploadProgress_->setAutoClose(false);
    uploadProgress_->setAutoReset(false);
    uploadProgress_->setValue(0);

    api_->uploadMedia(path, visibility);
}

void MainWindow::onSortChanged(int /*idx*/) {
    if (api_->isAuthenticated()) refreshFeed();
}

void MainWindow::onLogoutClicked() {
    statusBar()->showMessage("Çıkış yapılıyor…");
    api_->logout();
}

void MainWindow::onLogoutSucceeded() {
    statusBar()->showMessage("Çıkış yapıldı.");
    // Pencere geçişini main.cpp halleder (logoutSucceeded sinyaline abone)
}

void MainWindow::onLogoutFailed(int code, const QString& msg) {
    statusBar()->showMessage(QString("Çıkış hatası %1: %2").arg(code).arg(msg));
}

void MainWindow::onRefreshClicked() {
    if (pages_->currentIndex() == 0) refreshFeed();
    else                              refreshProfile();
}

void MainWindow::onChangePasswordClicked() {
    if (!api_->isAuthenticated()) return;
    QDialog dlg(this);
    dlg.setWindowTitle("Şifre değiştir");
    auto* form = new QFormLayout(&dlg);
    auto* oldPw = new QLineEdit;
    oldPw->setEchoMode(QLineEdit::Password);
    auto* newPw = new QLineEdit;
    newPw->setEchoMode(QLineEdit::Password);
    auto* newPw2 = new QLineEdit;
    newPw2->setEchoMode(QLineEdit::Password);
    form->addRow("Eski şifre:",       oldPw);
    form->addRow("Yeni şifre:",       newPw);
    form->addRow("Yeni şifre (tekrar):", newPw2);
    auto* status = new QLabel;
    status->setStyleSheet("color: #c0392b;");
    form->addRow(status);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText("Değiştir");
    bb->button(QDialogButtonBox::Cancel)->setText("Vazgeç");
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, [&]() {
        const QString op = oldPw->text();
        const QString np = newPw->text();
        if (np != newPw2->text()) { status->setText("Yeni şifreler eşleşmiyor."); return; }
        if (np.size() < 6)         { status->setText("Yeni şifre en az 6 karakter olmalı."); return; }
        if (np.contains(' '))       { status->setText("Şifre boşluk içeremez."); return; }
        dlg.accept();
        api_->changePassword(op, np);
    });
    dlg.exec();
}

// ----- Side tab slotları -----

void MainWindow::onShowFeed() {
    pages_->setCurrentIndex(0);
    refreshFeed();
}

void MainWindow::onShowProfile() {
    pages_->setCurrentIndex(1);
    refreshGreeting();
    refreshProfile();
}

// ----- LIST / USER_MEDIA cevapları -----

void MainWindow::onListSucceeded(const QList<MediaInfo>& items) {
    renderItems(feedGrid_, feedRow_, feedCol_, items, true);
    feedEmptyLbl_->setVisible(items.isEmpty());
    statusBar()->showMessage(QString("Feed: %1 medya.").arg(items.size()), 2500);
}

void MainWindow::onListFailed(int code, const QString& msg) {
    showError("Feed hatası", code, msg);
}

void MainWindow::onUserMediaSucceeded(const QList<MediaInfo>& items) {
    renderItems(profGrid_, profRow_, profCol_, items, true);
    profStatLbl_->setText(QString("Yüklediği medya: %1").arg(items.size()));
    profEmptyLbl_->setVisible(items.isEmpty());
    statusBar()->showMessage(QString("Profil: %1 medya.").arg(items.size()), 2500);
}

void MainWindow::onUserMediaFailed(int code, const QString& msg) {
    showError("Profil hatası", code, msg);
}

// ----- Preview / Download -----

void MainWindow::onPreviewSucceeded(const QString& mediaId, const QByteArray& data) {
    QPixmap pm;
    if (!pm.loadFromData(data)) return;
    thumbCache_.insert(mediaId, pm);
    const auto cards = waitingThumbs_.take(mediaId);
    for (const auto& cp : cards) {
        if (cp) cp->setThumbnail(pm);
    }
}

void MainWindow::onPreviewFailed(const QString& mediaId, int /*code*/, const QString& /*msg*/) {
    waitingThumbs_.remove(mediaId);
    // Placeholder kalmış olur, sorun değil
}

void MainWindow::onDownloadSucceeded(const QString& mediaId, const QString& type, const QByteArray& data) {
    if (type == "image") {
        auto* iv = new ImageViewer(this);
        iv->setAttribute(Qt::WA_DeleteOnClose);
        iv->setHeader(filenameOf(mediaId), ownerOf(mediaId));
        if (!iv->setImageFromBytes(data)) {
            delete iv;
            QMessageBox::warning(this, "Görüntüle", "Resim çözümlenemedi.");
            return;
        }
        iv->show();
    } else {
        auto* vv = new VideoViewer(this);
        vv->setAttribute(Qt::WA_DeleteOnClose);
        vv->setHeader(filenameOf(mediaId), ownerOf(mediaId));
        const QString ext = fileExtensionFor(mediaId);
        if (!vv->playFromBytes(data, ext.isEmpty() ? "mp4" : ext)) {
            delete vv;
            QMessageBox::warning(this, "Video", "Video açılamadı.");
            return;
        }
        vv->show();
    }
    statusBar()->showMessage("İndirildi.", 2000);
}

void MainWindow::onDownloadFailed(const QString& /*mediaId*/, int code, const QString& msg) {
    showError("İndirme hatası", code, msg);
}

// ----- Upload -----

void MainWindow::onUploadProgress(qint64 sent, qint64 total) {
    if (!uploadProgress_ || total <= 0) return;
    const int pct = static_cast<int>((sent * 100) / total);
    uploadProgress_->setValue(pct);
    uploadProgress_->setLabelText(QString("\"%1\" yükleniyor… %2 / %3 KB")
                                      .arg(uploadingFileName_).arg(sent / 1024).arg(total / 1024));
}

void MainWindow::onUploadSucceeded(const QString& mediaId) {
    if (uploadProgress_) { uploadProgress_->setValue(100); uploadProgress_->close(); uploadProgress_->deleteLater(); uploadProgress_ = nullptr; }
    statusBar()->showMessage(QString("Yüklendi: %1").arg(mediaId), 3000);
    refreshFeed();
    if (pages_->currentIndex() == 1) refreshProfile();
}

void MainWindow::onUploadFailed(int code, const QString& msg) {
    if (uploadProgress_) { uploadProgress_->close(); uploadProgress_->deleteLater(); uploadProgress_ = nullptr; }
    showError("Yükleme hatası", code, msg);
}

// ----- Like / Delete -----

void MainWindow::onCardLikeToggled(const QString& mediaId, bool nowLiked) {
    // Card kendini optimistic güncelledi; server cevabı gelince düzelt.
    if (nowLiked) api_->like(mediaId);
    else          api_->unlike(mediaId);
}

void MainWindow::onLikeSucceeded(const QString& mediaId, int newCount, bool nowLiked) {
    // Tüm grid'lerde bu id'ye sahip kartları bul ve güncelle
    const auto refresh = [&](QGridLayout* grid){
        for (int i = 0; i < grid->count(); ++i) {
            if (auto* c = qobject_cast<MediaCard*>(grid->itemAt(i)->widget())) {
                if (c->mediaId() == mediaId) {
                    c->setLikeCount(newCount);
                    c->setLikedByMe(nowLiked);
                }
            }
        }
    };
    if (feedGrid_) refresh(feedGrid_);
    if (profGrid_) refresh(profGrid_);
}

void MainWindow::onLikeFailed(const QString& mediaId, int code, const QString& msg) {
    // Optimistic değişikliği geri al
    const auto revert = [&](QGridLayout* grid){
        for (int i = 0; i < grid->count(); ++i) {
            if (auto* c = qobject_cast<MediaCard*>(grid->itemAt(i)->widget())) {
                if (c->mediaId() == mediaId) {
                    // Lokal state'i tersine çevir; tam doğru sayı için refresh çağır
                }
            }
        }
    };
    Q_UNUSED(revert);
    showError("Beğeni hatası", code, msg);
    refreshFeed();
}

void MainWindow::onCardDeleteRequested(const QString& mediaId) {
    const QString fname = filenameOf(mediaId);
    const auto ans = QMessageBox::question(
        this, "Sil",
        QString("\"%1\" medyasını silmek istediğine emin misin?").arg(fname),
        QMessageBox::Yes | QMessageBox::No);
    if (ans != QMessageBox::Yes) return;
    api_->deleteMedia(mediaId);
}

void MainWindow::onDeleteSucceeded(const QString& mediaId) {
    statusBar()->showMessage("Silindi.", 2000);
    thumbCache_.remove(mediaId);
    mediaIndex_.remove(mediaId);
    refreshFeed();
    if (pages_->currentIndex() == 1) refreshProfile();
}

void MainWindow::onDeleteFailed(const QString& /*mediaId*/, int code, const QString& msg) {
    showError("Silme hatası", code, msg);
}

// ----- Change password -----

void MainWindow::onChangePasswordSucceeded(const QString& /*newToken*/) {
    QMessageBox::information(this, "Şifre", "Şifre güncellendi. Yeni token kaydedildi.");
}

void MainWindow::onChangePasswordFailed(int code, const QString& msg) {
    showError("Şifre değiştirme hatası", code, msg);
}

// ----- Card open -----

void MainWindow::onCardOpenRequested(const QString& mediaId) {
    statusBar()->showMessage("İndiriliyor…");
    api_->downloadMedia(mediaId);
}

// ----- Yardımcılar -----

QString MainWindow::filenameOf(const QString& mediaId) const {
    return mediaIndex_.value(mediaId).filename;
}

QString MainWindow::ownerOf(const QString& mediaId) const {
    return mediaIndex_.value(mediaId).owner;
}

bool MainWindow::isVideo(const QString& mediaId) const {
    return mediaIndex_.value(mediaId).type == "video";
}

QString MainWindow::fileExtensionFor(const QString& mediaId) const {
    const QString fn = filenameOf(mediaId);
    return QFileInfo(fn).suffix().toLower();
}

void MainWindow::showError(const QString& title, int code, const QString& msg) {
    statusBar()->showMessage(QString("%1: %2 (kod %3)").arg(title, msg).arg(code), 5000);
    // 1001 → main.cpp tokenInvalidated ile login'e atılır (popup gerekmez)
    // 2002 → server "not implemented yet" — geliştirme süresi, popup spam'i olmasın
    // 0    → bağlantı yok — status bar yeter
    if (code != 1001 && code != 2002 && code != 0) {
        QMessageBox::warning(this, title, QString("Hata %1: %2").arg(code).arg(msg));
    }
}
