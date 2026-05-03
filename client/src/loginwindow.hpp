#pragma once
#include <QWidget>

class ApiClient;
class NetworkClient;
class QLineEdit;
class QLabel;
class QPushButton;
class QTabWidget;
class QTimer;

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    LoginWindow(ApiClient* api, NetworkClient* net, QWidget* parent = nullptr);

signals:
    void authenticated();   // ApiClient session set edildikten sonra

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLoginSucceeded(const QString& token);
    void onLoginFailed(int code, const QString& msg);
    void onRegisterSucceeded(const QString& token);
    void onRegisterFailed(int code, const QString& msg);
    void onConnected();
    void onDisconnected();
    void onNetworkError(const QString& msg);
    void onProtocolError(const QString& msg);
    void onResponseTimeout();

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    QWidget* buildLoginTab();
    QWidget* buildRegisterTab();
    void setBusy(bool b);
    QString validateUsername(const QString& u) const;
    QString validatePassword(const QString& p) const;

    ApiClient*     api_;
    NetworkClient* net_;
    QTabWidget*    tabs_;

    QLineEdit  *loginUser_, *loginPass_;
    QPushButton *loginBtn_;
    QLabel     *loginStatus_;

    QLineEdit  *regUser_, *regPass_, *regPass2_;
    QPushButton *regBtn_;
    QLabel     *regStatus_;

    QLabel     *connStatus_;
    QTimer*     responseTimer_ = nullptr;  // server cevap vermezse butonları geri aç
    bool        authDone_ = false;
};
