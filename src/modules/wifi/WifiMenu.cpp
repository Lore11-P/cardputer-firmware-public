#include "WifiMenu.h"

#include <Arduino.h>
#include <WiFi.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Config.h"
#include "Display.h"
#include "Logger.h"
#include "MenuManager.h"
#include "ModuleFlags.h"
#include "Navigation.h"
#include "Ui.h"

#if ENABLE_MODULE_WIFI

namespace wifi {
namespace {

struct Network {
    std::string ssid;
    std::string bssid;
    int rssi = 0;
    int channel = 1;
    int auth = 0;
};

const char* securityName(int auth) {
    switch (auth) {
        case WIFI_AUTH_OPEN:
            return "aperta";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2 Enterprise";
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2/WPA3";
        case WIFI_AUTH_WAPI_PSK:
            return "WAPI";
        default:
            return "sconosciuta";
    }
}

std::string shortened(const std::string& text, size_t maxLen) {
    if (text.size() <= maxLen) {
        return text;
    }
    return text.substr(0, maxLen - 1) + "~";
}

bool collectNetworks(std::vector<Network>& networks) {
    networks.clear();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(120);

    WiFi.scanNetworks(true, true);

    uint32_t started = millis();
    uint32_t lastDraw = 0;
    navigation::waitRelease();

    while (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
        navigation::update();
        if (navigation::poll().type == navigation::Event::Back) {
            WiFi.scanDelete();
            WiFi.mode(WIFI_OFF);
            return false;
        }
        const uint32_t now = millis();
        if (now - lastDraw > 120) {
            const int dots = static_cast<int>((now - started) / 350) % 4;
            display::clear();
            display::header("Scansione WiFi", "WiFi");
            display::textCentered(42, "Ricerca reti in corso" + std::string(dots, '.'),
                                  display::kAccent, 2);
            display::progressBar(20, 66, 200, 8,
                                 static_cast<float>(now - started) / 6000.0f,
                                 display::kInfo);
            display::textCentered(84, "Attendi qualche secondo", display::kTextDim, 0);
            display::footer("` = annulla");
            display::push();
            lastDraw = now;
        }
        delay(40);
    }

    const int count = WiFi.scanComplete();
    if (count < 0) {
        WiFi.scanDelete();
        WiFi.mode(WIFI_OFF);
        ui::message("WiFi Scanner", {"Scansione non riuscita."}, "WiFi");
        return false;
    }

    for (int i = 0; i < count; ++i) {
        Network network;
        network.ssid = WiFi.SSID(i).c_str();
        if (network.ssid.empty()) {
            network.ssid = "(nascosta)";
        }
        network.bssid = WiFi.BSSIDstr(i).c_str();
        network.rssi = WiFi.RSSI(i);
        network.channel = WiFi.channel(i);
        network.auth = static_cast<int>(WiFi.encryptionType(i));
        networks.push_back(network);
    }
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);

    std::sort(networks.begin(), networks.end(),
              [](const Network& a, const Network& b) { return a.rssi > b.rssi; });
    return true;
}

int chooseIndex(const std::string& title, const std::string& parent,
                const std::vector<std::string>& labels,
                const std::string& hint) {
    if (labels.empty()) {
        return -1;
    }
    constexpr int kHeaderH = 22;
    constexpr int kFooterH = 12;
    constexpr int kRowH = 20;
    const int visible =
        std::max(1, (display::height() - kHeaderH - kFooterH) / kRowH);
    int selected = 0;
    int top = 0;
    bool upHeld = false;
    bool downHeld = false;
    uint32_t nextUp = 0;
    uint32_t nextDown = 0;

    navigation::waitRelease();
    while (true) {
        navigation::update();
        const navigation::EventData ev = navigation::poll();
        const uint32_t now = millis();

        if (ev.type == navigation::Event::Back) {
            navigation::waitRelease();
            return -1;
        }
        if (ev.type == navigation::Event::Select) {
            navigation::waitRelease();
            return selected;
        }
        if (ev.type == navigation::Event::Up) {
            selected = (selected + static_cast<int>(labels.size()) - 1) %
                       static_cast<int>(labels.size());
        } else if (ev.type == navigation::Event::Down) {
            selected = (selected + 1) % static_cast<int>(labels.size());
        }

        const bool upNow = navigation::pressed(';');
        const bool downNow = navigation::pressed('.');
        if (upNow && !upHeld) {
            upHeld = true;
            nextUp = now + 380;
        } else if (!upNow) {
            upHeld = false;
        }
        if (downNow && !downHeld) {
            downHeld = true;
            nextDown = now + 380;
        } else if (!downNow) {
            downHeld = false;
        }
        if (upHeld && now >= nextUp) {
            selected = (selected + static_cast<int>(labels.size()) - 1) %
                       static_cast<int>(labels.size());
            nextUp = now + 110;
        }
        if (downHeld && now >= nextDown) {
            selected = (selected + 1) % static_cast<int>(labels.size());
            nextDown = now + 110;
        }

        if (selected < top) {
            top = selected;
        } else if (selected >= top + visible) {
            top = selected - visible + 1;
        }

        display::clear();
        display::header(title, parent);
        for (int row = 0; row < visible; ++row) {
            const int index = top + row;
            if (index >= static_cast<int>(labels.size())) {
                break;
            }
            const int y = kHeaderH + row * kRowH;
            const bool isSelected = index == selected;
            if (isSelected) {
                display::g().fillRoundRect(2, y + 1, display::width() - 6,
                                           kRowH - 2, 3, display::kAccentDark);
                display::g().fillRect(2, y + 6, 2, kRowH - 12,
                                      display::kAccent);
            }
            display::text(10, y + 6, shortened(labels[index], 28),
                          isSelected ? display::kWhite : display::kText, 2);
        }

        const int thumbH =
            std::max(12, visible * kRowH * visible /
                             static_cast<int>(labels.size()));
        const int trackY = kHeaderH;
        const int trackH = display::height() - kHeaderH - kFooterH;
        const int maxTop =
            std::max(1, static_cast<int>(labels.size()) - visible);
        const int thumbY =
            trackY + (trackH - thumbH) * top / maxTop;
        display::g().fillRect(display::width() - 3, trackY, 2, trackH,
                              display::kPanel);
        display::g().fillRect(display::width() - 3, thumbY, 2, thumbH,
                              display::kAccent);
        display::footer(hint);
        display::push();
        delay(16);
    }
}

void runScanner() {
    std::vector<Network> networks;
    if (!collectNetworks(networks)) {
        return;
    }
    if (networks.empty()) {
        ui::message("WiFi Scanner", {"Nessuna rete trovata."}, "WiFi");
        return;
    }

    MenuManager menu("Reti WiFi", "WiFi");
    for (const auto& network : networks) {
        std::string label =
            shortened(network.ssid, 16) + "  " + std::to_string(network.rssi) +
            "dBm" + (network.auth == WIFI_AUTH_OPEN ? "  *" : "");
        Network copy = network;
        menu.addItem(label, "wifi", [copy] {
            ui::message("Dettagli rete",
                        {"SSID: " + copy.ssid,
                         "BSSID: " + copy.bssid,
                         "Canale: " + std::to_string(copy.channel),
                         "Sicurezza: " + std::string(securityName(copy.auth)),
                         "RSSI: " + std::to_string(copy.rssi) + " dBm"},
                        "WiFi");
        });
    }
    menu.setHint("ENTER=dettagli  * = aperta  `=indietro");
    menu.run(true);
}


}  // namespace

void runMenu() {
    MenuManager menu("WiFi", "Menu principale");
    menu.addItem("Scanner reti", "scan", [] { runScanner(); });
    menu.setHint("`=indietro  ;.=nav  ENTER=apri");
    menu.run(true);
}

}  // namespace wifi

#endif  // ENABLE_MODULE_WIFI
