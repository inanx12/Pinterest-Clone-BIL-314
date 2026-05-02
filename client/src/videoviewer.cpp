#include "videoviewer.hpp"

#include <QAudioOutput>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QTemporaryFile>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

namespace {
QString fmtTime(qint64 ms) {
    if (ms < 0) ms = 0;
    QTime t(0, 0);
    t = t.addMSecs(static_cast<int>(ms));
    return (ms >= 3600000) ? t.toString("h:mm:ss") : t.toString("m:ss");
}
}  // namespace

VideoViewer::VideoViewer(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Video");
    resize(960, 720);
    setStyleSheet("background:#1d1d1d; color:#eee;");

    header_ = new QLabel(this);
    header_->setStyleSheet("color:#bbb; padding:6px;");

    video_ = new QVideoWidget(this);
    video_->setMinimumSize(640, 360);
    video_->setStyleSheet("background:#000;");

    player_ = new QMediaPlayer(this);
    audio_  = new QAudioOutput(this);
    player_->setAudioOutput(audio_);
    player_->setVideoOutput(video_);

    playBtn_ = new QPushButton("▶", this);
    playBtn_->setFixedWidth(40);
    playBtn_->setStyleSheet("background:#333; color:#eee; border:0; padding:6px;");

    seekSlider_ = new QSlider(Qt::Horizontal, this);
    timeLbl_    = new QLabel("0:00 / 0:00", this);
    timeLbl_->setStyleSheet("color:#bbb;");

    auto* close = new QPushButton("Kapat", this);
    close->setStyleSheet("background:#444; color:#eee; border:0; padding:6px 14px; border-radius:4px;");
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    auto* controls = new QHBoxLayout;
    controls->addWidget(playBtn_);
    controls->addWidget(seekSlider_, 1);
    controls->addWidget(timeLbl_);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->addWidget(header_);
    lay->addWidget(video_, 1);
    lay->addLayout(controls);
    lay->addWidget(close, 0, Qt::AlignRight);

    connect(playBtn_,    &QPushButton::clicked,           this, &VideoViewer::togglePlay);
    connect(seekSlider_, &QSlider::sliderMoved,           this, &VideoViewer::onSliderMoved);
    connect(player_,     &QMediaPlayer::positionChanged,  this, &VideoViewer::onPositionChanged);
    connect(player_,     &QMediaPlayer::durationChanged,  this, &VideoViewer::onDurationChanged);
}

VideoViewer::~VideoViewer() {
    if (player_) player_->stop();
    delete tempFile_;  // QTemporaryFile destructor dosyayı siler
}

bool VideoViewer::playFromBytes(const QByteArray& data, const QString& fileExtension) {
    delete tempFile_;
    tempFile_ = new QTemporaryFile(QDir::tempPath() + "/pcv_XXXXXX." + fileExtension);
    tempFile_->setAutoRemove(true);
    if (!tempFile_->open()) return false;
    tempFile_->write(data);
    tempFile_->flush();
    tempFile_->close();

    player_->setSource(QUrl::fromLocalFile(tempFile_->fileName()));
    player_->play();
    playBtn_->setText("⏸");
    return true;
}

void VideoViewer::setHeader(const QString& filename, const QString& owner) {
    header_->setText(QString("<b>%1</b>%2")
                         .arg(filename.toHtmlEscaped(),
                              owner.isEmpty() ? QString() : QString("  •  yükleyen: %1").arg(owner.toHtmlEscaped())));
}

void VideoViewer::togglePlay() {
    if (player_->playbackState() == QMediaPlayer::PlayingState) {
        player_->pause();
        playBtn_->setText("▶");
    } else {
        player_->play();
        playBtn_->setText("⏸");
    }
}

void VideoViewer::onPositionChanged(qint64 pos) {
    if (!seekSlider_->isSliderDown()) seekSlider_->setValue(static_cast<int>(pos));
    timeLbl_->setText(QString("%1 / %2").arg(fmtTime(pos), fmtTime(player_->duration())));
}

void VideoViewer::onDurationChanged(qint64 dur) {
    seekSlider_->setRange(0, static_cast<int>(dur));
    timeLbl_->setText(QString("%1 / %2").arg(fmtTime(player_->position()), fmtTime(dur)));
}

void VideoViewer::onSliderMoved(int pos) {
    player_->setPosition(pos);
}
