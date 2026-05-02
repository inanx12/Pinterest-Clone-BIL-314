#pragma once
#include <QByteArray>
#include <QDialog>

class QLabel;

// Tam boyut resim göstericisi. Byte buffer'dan QPixmap üretir.
class ImageViewer : public QDialog {
    Q_OBJECT
public:
    explicit ImageViewer(QWidget* parent = nullptr);

    bool setImageFromBytes(const QByteArray& data);
    void setHeader(const QString& filename, const QString& owner);

private:
    QLabel* header_;
    QLabel* image_;
};
