#include <QApplication>
#include <QSettings>
#include <QStringList>

#include "constants.hpp"
#include "networkclient.hpp"
#include "apiclient.hpp"
#include "loginwindow.hpp"
#include "mainwindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("BIL314");
    QApplication::setApplicationName("PinterestClone");

    NetworkClient net;
    net.setAutoReconnect(true, 2000);  // 2 sn'de bir tekrar dene

    ApiClient api(&net);

    LoginWindow loginWin(&api, &net);
    MainWindow  mainWin(&api);

    auto showLogin = [&] {
        mainWin.hide();
        loginWin.show();
        loginWin.raise();
        loginWin.activateWindow();
    };
    auto showMain = [&] {
        loginWin.hide();
        mainWin.show();
        mainWin.raise();
        mainWin.activateWindow();
    };

    QObject::connect(&loginWin, &LoginWindow::authenticated, [&] {
        QSettings s;
        s.setValue("auth/username", api.username());
        s.setValue("auth/token",    api.token());
        showMain();
        mainWin.onAuthenticated();
    });

    QObject::connect(&api, &ApiClient::logoutSucceeded, [&] {
        QSettings s;
        s.remove("auth/token");
        s.remove("auth/username");
        showLogin();
    });

    // ERR 1001 → token geçersiz → settings temizle, login'e dön
    QObject::connect(&api, &ApiClient::tokenInvalidated, [&] {
        QSettings s;
        s.remove("auth/token");
        s.remove("auth/username");
        showLogin();
    });

    // Yeniden bağlanınca, kayıtlı token varsa otomatik devam (sessiz).
    // Server "ilk istekte 1001 dönerse" mantığıyla protokol madde 7'ye uyuyoruz —
    // burada extra LOGIN göndermiyoruz; ilk korumalı istek 1001 alırsa tokenInvalidated
    // tetiklenip login'e dönülecek.

    // --ui-test: server'a bağlanmadan, sahte oturumla doğrudan MainWindow.
    // SADECE arayüz iskeletini görmek için. LIST/UPLOAD/DOWNLOAD bu modda çalışmaz
    // (server cevap vermez → komutlar queue'da takılır). Logout offline çalışır.
    const QStringList args = QApplication::arguments();
    if (args.contains("--ui-test")) {
        net.setAutoReconnect(false);  // 127.0.0.1:8080'e boşa bağlanmaya çalışmasın
        api.setSession("test_user", "fake_ui_test_token");
        showMain();
        return app.exec();
    }

    QSettings s;
    const QString savedUser  = s.value("auth/username").toString();
    const QString savedToken = s.value("auth/token").toString();
    if (!savedToken.isEmpty()) {
        api.setSession(savedUser, savedToken);
        showMain();
        // Bağlanır bağlanmaz feed yüklenmeye çalışılacak
        QObject::connect(&net, &NetworkClient::connected, &mainWin,
                         &MainWindow::onAuthenticated, Qt::SingleShotConnection);
    } else {
        showLogin();
    }

    net.connectToServer("178.105.95.17", pc::constants::SERVER_PORT);
    return app.exec();
}
