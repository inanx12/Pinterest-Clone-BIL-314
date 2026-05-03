#include "loginwindow.hpp"
#include "apiclient.hpp"
#include "networkclient.hpp"
#include "constants.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

LoginWindow::LoginWindow(ApiClient* api, NetworkClient* net, QWidget* parent)
    : QWidget(parent), api_(api), net_(net) {
    setWindowTitle("Pinterest-Clone — Giriş");
    resize(380, 300);

    tabs_ = new QTabWidget(this);
    tabs_->addTab(buildLoginTab(), "Giriş Yap");
    tabs_->addTab(buildRegisterTab(), "Kayıt Ol");

    connStatus_ = new QLabel("Sunucuya bağlanılıyor…", this);
    connStatus_->setStyleSheet("color: gray;");

    auto* root = new QVBoxLayout(this);
    root->addWidget(tabs_);
    root->addWidget(connStatus_);

    setBusy(true);

    connect(net_, &NetworkClient::connected,     this, &LoginWindow::onConnected);
    connect(net_, &NetworkClient::disconnected,  this, &LoginWindow::onDisconnected);
    connect(net_, &NetworkClient::errorOccurred, this, &LoginWindow::onNetworkError);

    connect(api_, &ApiClient::loginSucceeded,    this, &LoginWindow::onLoginSucceeded);
    connect(api_, &ApiClient::loginFailed,       this, &LoginWindow::onLoginFailed);
    connect(api_, &ApiClient::registerSucceeded, this, &LoginWindow::onRegisterSucceeded);
    connect(api_, &ApiClient::registerFailed,    this, &LoginWindow::onRegisterFailed);
    connect(api_, &ApiClient::protocolError,     this, &LoginWindow::onProtocolError);

    // Server cevap vermezse butonlar sonsuza kadar disabled kalmasın diye timeout.
    responseTimer_ = new QTimer(this);
    responseTimer_->setSingleShot(true);
    connect(responseTimer_, &QTimer::timeout, this, &LoginWindow::onResponseTimeout);

    if (net_->isConnected()) onConnected();
}

QWidget* LoginWindow::buildLoginTab() {
    auto* w = new QWidget;
    loginUser_ = new QLineEdit;
    loginUser_->setPlaceholderText("kullanıcı adı");
    loginPass_ = new QLineEdit;
    loginPass_->setPlaceholderText("şifre");
    loginPass_->setEchoMode(QLineEdit::Password);

    loginBtn_ = new QPushButton("Giriş Yap");
    loginBtn_->setDefault(true);
    loginStatus_ = new QLabel;
    loginStatus_->setWordWrap(true);
    loginStatus_->setStyleSheet("color: #c0392b;");

    auto* form = new QFormLayout;
    form->addRow("Kullanıcı:", loginUser_);
    form->addRow("Şifre:",     loginPass_);

    auto* lay = new QVBoxLayout(w);
    lay->addLayout(form);
    lay->addWidget(loginBtn_);
    lay->addWidget(loginStatus_);
    lay->addStretch();

    connect(loginBtn_,  &QPushButton::clicked,    this, &LoginWindow::onLoginClicked);
    connect(loginPass_, &QLineEdit::returnPressed, loginBtn_, &QPushButton::click);
    connect(loginUser_, &QLineEdit::returnPressed, loginBtn_, &QPushButton::click);
    return w;
}

QWidget* LoginWindow::buildRegisterTab() {
    auto* w = new QWidget;
    regUser_  = new QLineEdit;
    regUser_->setPlaceholderText("3-20 karakter, sadece harf/rakam/_");
    regPass_  = new QLineEdit;
    regPass_->setPlaceholderText("min 6 karakter");
    regPass_->setEchoMode(QLineEdit::Password);
    regPass2_ = new QLineEdit;
    regPass2_->setPlaceholderText("şifre tekrar");
    regPass2_->setEchoMode(QLineEdit::Password);

    regBtn_ = new QPushButton("Kayıt Ol");
    regStatus_ = new QLabel;
    regStatus_->setWordWrap(true);
    regStatus_->setStyleSheet("color: #c0392b;");

    auto* form = new QFormLayout;
    form->addRow("Kullanıcı:",     regUser_);
    form->addRow("Şifre:",          regPass_);
    form->addRow("Şifre (tekrar):", regPass2_);

    auto* lay = new QVBoxLayout(w);
    lay->addLayout(form);
    lay->addWidget(regBtn_);
    lay->addWidget(regStatus_);
    lay->addStretch();

    connect(regBtn_,   &QPushButton::clicked,     this, &LoginWindow::onRegisterClicked);
    connect(regPass2_, &QLineEdit::returnPressed, regBtn_, &QPushButton::click);
    return w;
}

void LoginWindow::setBusy(bool b) {
    loginBtn_->setEnabled(!b);
    regBtn_->setEnabled(!b);
    if (responseTimer_) {
        if (b) responseTimer_->start(10000);  // 10 sn server cevap vermezse butonlar açılsın
        else   responseTimer_->stop();
    }
}

QString LoginWindow::validateUsername(const QString& u) const {
    using namespace pc::constants;
    if (u.size() < USERNAME_MIN || u.size() > USERNAME_MAX)
        return QString("Kullanıcı adı %1-%2 karakter olmalı.").arg(USERNAME_MIN).arg(USERNAME_MAX);
    static const QRegularExpression rx("^[a-zA-Z0-9_]+$");
    if (!rx.match(u).hasMatch())
        return "Sadece harf, rakam ve _ kullanılabilir.";
    return {};
}

QString LoginWindow::validatePassword(const QString& p) const {
    using namespace pc::constants;
    if (p.size() < PASSWORD_MIN)
        return QString("Şifre en az %1 karakter olmalı.").arg(PASSWORD_MIN);
    if (p.contains(' '))
        return "Şifre boşluk içeremez.";
    return {};
}

void LoginWindow::onLoginClicked() {
    loginStatus_->clear();
    const QString u = loginUser_->text().trimmed();
    const QString p = loginPass_->text();

    QString err = validateUsername(u);
    if (err.isEmpty()) err = validatePassword(p);
    if (!err.isEmpty()) { loginStatus_->setText(err); return; }
    if (!net_->isConnected()) {
        loginStatus_->setText("Sunucuya bağlı değil.");
        return;
    }

    setBusy(true);
    api_->login(u, p);
}

void LoginWindow::onRegisterClicked() {
    regStatus_->clear();
    const QString u  = regUser_->text().trimmed();
    const QString p  = regPass_->text();
    const QString p2 = regPass2_->text();

    QString err = validateUsername(u);
    if (err.isEmpty()) err = validatePassword(p);
    if (err.isEmpty() && p != p2) err = "Şifreler eşleşmiyor.";
    if (!err.isEmpty()) { regStatus_->setText(err); return; }
    if (!net_->isConnected()) {
        regStatus_->setText("Sunucuya bağlı değil.");
        return;
    }

    setBusy(true);
    api_->registerUser(u, p);
}

void LoginWindow::onLoginSucceeded(const QString&) {
    setBusy(false);
    authDone_ = true;
    emit authenticated();
}

void LoginWindow::onLoginFailed(int code, const QString& msg) {
    setBusy(false);
    loginStatus_->setText(QString("Hata %1: %2").arg(code).arg(msg.isEmpty() ? "Giriş başarısız." : msg));
}

void LoginWindow::onRegisterSucceeded(const QString&) {
    setBusy(false);
    authDone_ = true;
    emit authenticated();
}

void LoginWindow::onRegisterFailed(int code, const QString& msg) {
    setBusy(false);
    regStatus_->setText(QString("Hata %1: %2").arg(code).arg(msg.isEmpty() ? "Kayıt başarısız." : msg));
}

void LoginWindow::onConnected() {
    connStatus_->setText("Sunucu bağlı.");
    connStatus_->setStyleSheet("color: #27ae60;");
    setBusy(false);
}

void LoginWindow::onDisconnected() {
    connStatus_->setText("Sunucu bağlantısı koptu.");
    connStatus_->setStyleSheet("color: #c0392b;");
    setBusy(true);
}

void LoginWindow::onNetworkError(const QString& msg) {
    connStatus_->setText("Ağ hatası: " + msg);
    connStatus_->setStyleSheet("color: #c0392b;");
    setBusy(true);
}

void LoginWindow::onProtocolError(const QString& msg) {
    // Server bozuk format gönderdi (örn. "OK" sonrası boşluk yok, \n eksik vs.)
    // Butonları geri aç ki kullanıcı tekrar denesin.
    setBusy(false);
    const QString text = "Server cevabı bozuk: " + msg;
    loginStatus_->setText(text);
    regStatus_->setText(text);
}

void LoginWindow::onResponseTimeout() {
    // Server hiç cevap vermedi (10 sn). Butonları geri aç.
    setBusy(false);
    const QString text = "Sunucu cevap vermedi (10 sn). Tekrar dene.";
    loginStatus_->setText(text);
    regStatus_->setText(text);
}

void LoginWindow::closeEvent(QCloseEvent* e) {
    // LoginWindow auth olmadan kapatılırsa app çıksın.
    if (!authDone_) QApplication::quit();
    QWidget::closeEvent(e);
}
