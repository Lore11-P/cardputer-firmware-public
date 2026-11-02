#include "BleScanner.h"

#include <Arduino.h>
#include <NimBLEDevice.h>


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

#if ENABLE_MODULE_BLE

namespace ble {
namespace {

struct ScanResult {
    std::string name;
    std::string address;
    int rssi = 0;
};

std::vector<ScanResult> g_results;

class ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
public:
    void onResult(NimBLEAdvertisedDevice* device) override {
        if (device == nullptr) {
            return;
        }
        const std::string address = device->getAddress().toString();
        for (auto& existing : g_results) {
            if (existing.address == address) {
                existing.rssi = std::max(existing.rssi, device->getRSSI());
                if (device->haveName()) {
                    existing.name = device->getName();
                }
                return;
            }
        }
        if (g_results.size() >= 40) {
            return;
        }
        ScanResult result;
        result.address = address;
        result.name = device->haveName() ? device->getName() : "(senza nome)";
        result.rssi = device->getRSSI();
        g_results.push_back(result);
    }
};

void ensureBle() {
    if (!NimBLEDevice::getInitialized()) {
        NimBLEDevice::init("CardputerFW");
    }
}

void drawScanProgress(int round, int total, size_t found, bool interrupted) {
    display::clear();
    display::header("BLE Scanner", "BLE");
    display::textCentered(36, interrupted ? "Scansione interrotta" : "Scansione in corso",
                          interrupted ? display::kWarning : display::kAccent, 2);
    display::progressBar(20, 58, 200, 8,
                         static_cast<float>(round) / std::max(total, 1));
    display::textCentered(76, "Dispositivi trovati: " + std::to_string(found),
                          display::kText, 2);
    display::textCentered(96, "Avvicina i dispositivi per il segnale",
                          display::kTextDim, 0);
    display::footer("` = interrompi");
    display::push();
}

void runScanner() {
    g_results.clear();
    ensureBle();

    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scan == nullptr) {
        ui::message("BLE Scanner", {"!Stack BLE non disponibile.", "Riavvia il dispositivo."}, "BLE");
        return;
    }

    ScanCallbacks callbacks;
    scan->setAdvertisedDeviceCallbacks(&callbacks, true);
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(99);
    scan->stop();  // se una scansione precedente e' rimasta appesa

    constexpr int kRounds = 4;
    // Finestre piu' lunghe: lo scan attivo ha bisogno di tempo per ricevere le
    // SCAN_RESPONSE, che e' dove la maggior parte dei dispositivi mette il nome.
    constexpr uint32_t kRoundMs = 1500;
    // Margine oltre il quale consideriamo la finestra persa e la chiudiamo noi:
    // la variante bloccante di start() aspetta una notifica che, se il
    // controller non completa (o risponde BLE_HS_EALREADY), non arriva mai.
    constexpr uint32_t kGraceMs = 1500;

    bool aborted = false;
    navigation::waitRelease();
    for (int round = 0; round < kRounds && !aborted; ++round) {
        drawScanProgress(round, kRounds, g_results.size(), false);
        scan->start(kRoundMs, nullptr, false);  // non bloccante: si esce subito
        const uint32_t deadline = millis() + kRoundMs + kGraceMs;
        while (scan->isScanning() && millis() < deadline) {
            navigation::update();
            if (navigation::poll().type == navigation::Event::Back) {
                aborted = true;
                break;
            }
            delay(20);
        }
        if (scan->isScanning()) {
            scan->stop();  // finestra chiusa a forza
        }
        scan->clearResults();
    }

    if (aborted) {
        logger::info("BLE scanner interrotto dall'utente");
    }
    logger::info("BLE scanner: %u dispositivi, stack=%s heap=%u",
                 static_cast<unsigned>(g_results.size()),
                 NimBLEDevice::getInitialized() ? "ok" : "non init",
                 static_cast<unsigned>(ESP.getFreeHeap()));

    if (g_results.empty()) {
        ui::message("BLE Scanner",
                    {aborted ? "!Scansione interrotta." : "!Nessun dispositivo rilevato.",
                     "Verifica che il BLE sia attivo nelle vicinanze.",
                     "Se il problema persiste, riavvia il dispositivo."},
                    "BLE");
        return;
    }

    std::sort(g_results.begin(), g_results.end(),
              [](const ScanResult& a, const ScanResult& b) {
                  return a.rssi > b.rssi;
              });

    MenuManager menu("Dispositivi BLE", "BLE");
    for (const auto& result : g_results) {
        std::string name = result.name;
        if (name.size() > 18) {
            name = name.substr(0, 18) + "~";
        }
        const std::string label =
            name + "  " + std::to_string(result.rssi) + "dBm";
        ScanResult copy = result;
        menu.addItem(label, "ble", [copy] {
            ui::message("Dettagli BLE",
                        {"Nome: " + copy.name,
                         "MAC: " + copy.address,
                         "RSSI: " + std::to_string(copy.rssi) + " dBm"},
                        "BLE");
        });
    }
    menu.setHint("ENTER=dettagli  `=indietro");
    menu.run(true);
}




}  // namespace

void runMenu() {
    MenuManager menu("BLE", "Menu principale");
    menu.addItem("Scanner BLE", "scan", [] { runScanner(); });
    menu.setHint("`=indietro  ;.=nav  ENTER=apri");
    menu.run(true);
}

}  // namespace ble

#endif  // ENABLE_MODULE_BLE
