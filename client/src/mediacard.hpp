#pragma once
#include <QFrame>

class QLabel;
class QPushButton;
class QPixmap;

// Feed/profil grid'inde tek bir medya kartı.
// Thumbnail + filename + uploader + like + visibility + delete (owner ise).
class MediaCard : public QFrame {
    Q_OBJECT
public:
    enum class Type { Image, Video };
    enum class Visibility { Public, Private };

    explicit MediaCard(QWidget* parent = nullptr);

    void setMediaId(const QString& id)        { mediaId_ = id; }
    void setFilename(const QString& name);
    void setOwner(const QString& owner);
    void setType(Type t);
    void setVisibility(Visibility v);
    void setLikeCount(int n);
    void setLikedByMe(bool liked);
    void setOwnedByMe(bool owned);            // delete butonunu görünür yapar
    void setThumbnail(const QPixmap& pm);     // boş pm => placeholder

    QString mediaId() const { return mediaId_; }
    QString owner()   const { return owner_; }

signals:
    void openRequested(const QString& mediaId);
    void likeToggled(const QString& mediaId, bool nowLiked);
    void deleteRequested(const QString& mediaId);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private slots:
    void onLikeClicked();
    void onDeleteClicked();

private:
    void rebuildLikeText();
    void rebuildVisibilityBadge();
    void rebuildTypeBadge();
    void renderPlaceholderThumb();

    QString mediaId_;
    QString filename_;
    QString owner_;
    Type       type_       = Type::Image;
    Visibility visibility_ = Visibility::Public;
    int  likeCount_  = 0;
    bool likedByMe_  = false;
    bool ownedByMe_  = false;

    QLabel*      thumb_;
    QLabel*      typeBadge_;        // ▶ overlay (video)
    QLabel*      visibilityBadge_;  // 🔒 overlay (private, sadece owner)
    QLabel*      filenameLbl_;
    QLabel*      ownerLbl_;
    QPushButton* likeBtn_;
    QPushButton* deleteBtn_;
};
