#include "protocol.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace pc::proto {

// clientdan gelen veriyi ayrıştırarak anlamlı bir komuta dönüştürür. 
// Örnek: "LOGIN user pass" metni gelirse komut adını "LOGIN", argümanları da [user, pass] olarak ayırır.
Command parse_command(const std::string& line) {
    Command cmd;
    if (line.empty()) return cmd;

    std::istringstream iss(line);
    std::string token;

    // İlk kelime = komut adı. Büyük/küçük harf duyarsız olması için tamamen büyük harfe çeviriyoruz.
    if (iss >> token) {
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        cmd.name = token;
    }

    // case-sensitive — username, şifre, token vs. yapısı bozulmadan listeye eklenir.
    while (iss >> token) {
        cmd.args.push_back(token);
    }

    return cmd;
}

// cliente işlemin sorunsuz tamamlandığını bildiren standart bir başarılı yanıtı döndürür.
std::string ok() {
    return "OK\n";
}

std::string ready() {
    return "READY\n";
}

// cliente işlemin başarılı olduğunu bildirirken, ek bir veri döndürür. 
// Örnek kullanım: Giriş yapıldığında token göndermek için "OK <token_değeri>" üretmek.
std::string ok(const std::string& payload) {
    return "OK " + payload + "\n";
}

// İstemciye bir şeylerin ters gittiğini, hata kodunu ve hatanın açıklamasını standart bir formatta ("ERR") döndürür.
// Örnek kullanım: Yanlış şifre girildiğinde "ERR 401 Hatali sifre" yanıtı üretmek.
std::string err(int code, const std::string& message) {
    return "ERR " + std::to_string(code) + " " + message + "\n";
}

}
