#include "imageviewer.hpp"

#include <QGuiApplication>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

ImageViewer::ImageViewer(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Görüntüle");
    resize(900, 700);
    setStyleSheet("background:#2b2b2b; color:#eee;");

    header_ = new QLabel(this);
    header_->setStyleSheet("color:#bbb; padding:6px;");

    image_ = new QLabel(this);
    image_->setAlignment(Qt::AlignCenter);
    image_->setStyleSheet("background:#1d1d1d;");
    image_->setMinimumSize(400, 300);

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(image_);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* close = new QPushButton("Kapat", this);
    close->setStyleSheet("background:#444; color:#eee; border:0; padding:6px 14px; border-radius:4px;");
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->addWidget(header_);
    lay->addWidget(scroll, 1);
    lay->addWidget(close, 0, Qt::AlignRight);
}

bool ImageViewer::setImageFromBytes(const QByteArray& data) {
    QPixmap pm;
    if (!pm.loadFromData(data)) return false;

    // Ekrana göre makul ölçek (çok büyük resim viewer'ı patlamasın).
    const QSize scr = QGuiApplication::primaryScreen()->availableSize();
    const QSize cap(scr.width() * 8 / 10, scr.height() * 8 / 10);
    if (pm.width() > cap.width() || pm.height() > cap.height()) {
        pm = pm.scaled(cap, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    image_->setPixmap(pm);
    image_->setFixedSize(pm.size());
    return true;
}

void ImageViewer::setHeader(const QString& filename, const QString& owner) {
    header_->setText(QString("<b>%1</b>%2")
                         .arg(filename.toHtmlEscaped(),
                              owner.isEmpty() ? QString() : QString("  •  yükleyen: %1").arg(owner.toHtmlEscaped())));
}
