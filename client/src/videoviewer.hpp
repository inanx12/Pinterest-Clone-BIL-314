#pragma once
#include <QByteArray>
#include <QDialog>
#include <QString>

class QLabel;
class QMediaPlayer;
class QAudioOutput;
class QVideoWidget;
class QPushButton;
class QSlider;
class QTemporaryFile;

// İndirilen video byte buffer'ını temp dosyasına yazıp QMediaPlayer ile oynatır.
class VideoViewer : public QDialog {
    Q_OBJECT
public:
    explicit VideoViewer(QWidget* parent = nullptr);
    ~VideoViewer() override;

    // Byte buffer'ı temp dosyaya yazar ve oynatmaya başlar. Başarısızsa false.
    bool playFromBytes(const QByteArray& data, const QString& fileExtension);
    void setHeader(const QString& filename, const QString& owner);

private slots:
    void togglePlay();
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);
    void onSliderMoved(int pos);

private:
    QLabel*         header_;
    QMediaPlayer*   player_;
    QAudioOutput*   audio_;
    QVideoWidget*   video_;
    QPushButton*    playBtn_;
    QSlider*        seekSlider_;
    QLabel*         timeLbl_;
    QTemporaryFile* tempFile_ = nullptr;
};
