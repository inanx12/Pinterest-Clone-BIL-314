#include "mediacard.hpp"

#include <QEnterEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {
constexpr int kThumbW = 200;
constexpr int kThumbH = 200;
}  // namespace

MediaCard::MediaCard(QWidget* parent) : QFrame(parent) {
    setObjectName("mediaCard");
    setFrameShape(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor);
    setFixedWidth(kThumbW + 24);

    // Thumbnail container (fixed size, overlay'ler için)
    auto* thumbHolder = new QWidget(this);
    thumbHolder->setFixedSize(kThumbW, kThumbH);

    thumb_ = new QLabel(thumbHolder);
    thumb_->setObjectName("cardThumb");
    thumb_->setGeometry(0, 0, kThumbW, kThumbH);
    thumb_->setAlignment(Qt::AlignCenter);
    thumb_->setScaledContents(false);
    renderPlaceholderThumb();

    typeBadge_ = new QLabel("▶", thumbHolder);
    typeBadge_->setObjectName("typeBadge");
    typeBadge_->setAlignment(Qt::AlignCenter);
    typeBadge_->setFixedSize(28, 28);
    typeBadge_->move(8, 8);
    typeBadge_->hide();

    visibilityBadge_ = new QLabel("🔒", thumbHolder);
    visibilityBadge_->setObjectName("visibilityBadge");
    visibilityBadge_->setAlignment(Qt::AlignCenter);
    visibilityBadge_->setFixedSize(28, 28);
    visibilityBadge_->move(kThumbW - 28 - 8, 8);
    visibilityBadge_->hide();

    // Bilgi satırları
    filenameLbl_ = new QLabel(this);
    filenameLbl_->setObjectName("cardFilename");
    filenameLbl_->setWordWrap(false);
    filenameLbl_->setTextInteractionFlags(Qt::NoTextInteraction);

    ownerLbl_ = new QLabel(this);
    ownerLbl_->setObjectName("cardOwner");

    // Aksiyonlar (like / delete)
    likeBtn_ = new QPushButton(this);
    likeBtn_->setObjectName("likeBtn");
    likeBtn_->setCursor(Qt::PointingHandCursor);
    likeBtn_->setFlat(true);
    rebuildLikeText();

    deleteBtn_ = new QPushButton("Sil", this);
    deleteBtn_->setObjectName("deleteBtn");
    deleteBtn_->setCursor(Qt::PointingHandCursor);
    deleteBtn_->setFlat(true);
    deleteBtn_->hide();

    auto* actions = new QHBoxLayout;
    actions->setContentsMargins(0, 0, 0, 0);
    actions->setSpacing(6);
    actions->addWidget(likeBtn_);
    actions->addStretch();
    actions->addWidget(deleteBtn_);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(4);
    root->addWidget(thumbHolder, 0, Qt::AlignHCenter);
    root->addWidget(filenameLbl_);
    root->addWidget(ownerLbl_);
    root->addLayout(actions);

    connect(likeBtn_,   &QPushButton::clicked, this, &MediaCard::onLikeClicked);
    connect(deleteBtn_, &QPushButton::clicked, this, &MediaCard::onDeleteClicked);
}

void MediaCard::setFilename(const QString& name) {
    filename_ = name;
    QString shown = name;
    if (shown.size() > 22) shown = shown.left(20) + "…";
    filenameLbl_->setText(shown);
    filenameLbl_->setToolTip(name);
}

void MediaCard::setOwner(const QString& owner) {
    owner_ = owner;
    ownerLbl_->setText(owner.isEmpty() ? QString() : QString("yükleyen: %1").arg(owner));
}

void MediaCard::setType(Type t) {
    type_ = t;
    rebuildTypeBadge();
}

void MediaCard::setVisibility(Visibility v) {
    visibility_ = v;
    rebuildVisibilityBadge();
}

void MediaCard::setLikeCount(int n) {
    likeCount_ = n < 0 ? 0 : n;
    rebuildLikeText();
}

void MediaCard::setLikedByMe(bool liked) {
    likedByMe_ = liked;
    rebuildLikeText();
}

void MediaCard::setOwnedByMe(bool owned) {
    ownedByMe_ = owned;
    deleteBtn_->setVisible(owned);
    rebuildVisibilityBadge();
}

void MediaCard::setThumbnail(const QPixmap& pm) {
    if (pm.isNull()) {
        renderPlaceholderThumb();
    } else {
        thumb_->setPixmap(pm.scaled(kThumbW, kThumbH,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    }
}

void MediaCard::rebuildLikeText() {
    const QChar heart = likedByMe_ ? QChar(0x2665) : QChar(0x2661);  // ♥ vs ♡
    likeBtn_->setText(QString("%1 %2").arg(heart).arg(likeCount_));
    likeBtn_->setProperty("liked", likedByMe_);
    likeBtn_->style()->unpolish(likeBtn_);
    likeBtn_->style()->polish(likeBtn_);
}

void MediaCard::rebuildVisibilityBadge() {
    // 🔒 sadece kendi private medyalarımda göster
    const bool show = ownedByMe_ && visibility_ == Visibility::Private;
    visibilityBadge_->setVisible(show);
}

void MediaCard::rebuildTypeBadge() {
    typeBadge_->setVisible(type_ == Type::Video);
}

void MediaCard::renderPlaceholderThumb() {
    QPixmap pm(kThumbW, kThumbH);
    pm.fill(QColor("#e8e8e8"));
    QPainter p(&pm);
    p.setPen(QColor("#b0b0b0"));
    p.setFont(QFont("sans-serif", 28, QFont::DemiBold));
    p.drawText(pm.rect(), Qt::AlignCenter, "Resim");
    thumb_->setPixmap(pm);
}

void MediaCard::onLikeClicked() {
    likedByMe_ = !likedByMe_;
    likeCount_ += likedByMe_ ? 1 : -1;
    if (likeCount_ < 0) likeCount_ = 0;
    rebuildLikeText();
    emit likeToggled(mediaId_, likedByMe_);
}

void MediaCard::onDeleteClicked() {
    emit deleteRequested(mediaId_);
}

void MediaCard::mousePressEvent(QMouseEvent* e) {
    QFrame::mousePressEvent(e);
    if (e->button() == Qt::LeftButton) {
        // Action butonlarına basıldıysa zaten o slotlar tetiklenir; geri kalan
        // alana basınca medyayı aç.
        const QWidget* w = childAt(e->pos());
        if (w == likeBtn_ || w == deleteBtn_) return;
        emit openRequested(mediaId_);
    }
}

void MediaCard::enterEvent(QEnterEvent* e) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(e);
}

void MediaCard::leaveEvent(QEvent* e) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(e);
}
