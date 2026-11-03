# CardputerFirmware v1.0

Firmware modulare per **M5Stack Cardputer ADV** (ESP32-S3FN8, 8 MB flash, **senza PSRAM**), scritto in C++17 con PlatformIO/Arduino. Interfaccia a menu con icone, navigazione da tastiera e moduli attivabili/disattivabili a runtime.

Questa è la **build pubblica**: include scanner BLE/Wi-Fi, radar passivo, web radio, telecomandi IR, giochi e gestione microSD. Non include funzioni offensive.

## Stato dei moduli

| Componente | Stato | Note |
|---|---|---|
| Menu principale / UI / navigazione | ✅ | Carosello orizzontale a icone con animazione, griglia, breadcrumb e liste scrollabili |
| Impostazioni | ✅ | Luminosità, rotazione, spegnimento automatico, info dispositivo, rete WiFi, moduli on/off, export/import JSON, reset |
| Giochi | ✅ | Tetris, Breakout, Doom Lite, Snake, Pong, 2048, Flappy, Simon, Minesweeper e Connect 4 con record su SD |
| Infrarossi | ✅ | Parser `.ir` Flipper, ricerca per nome, preferiti, ripresa ultimo telecomando, tasti rapidi, scanner Power, TV-B-Gone |
| BLE | ✅ | Scanner passivo/attivo con nome, MAC e RSSI |
| Wi-Fi | ✅ | Scanner reti con dettagli e configurazione della connessione |
| Radar Wi-Fi/BLE | ✅ | Finder BLE hot/cold con HUD stile contatore Geiger, radar PPI, vista banda 2.4 GHz con TDM Wi-Fi/BLE, presenza CSI sperimentale |
| Web Radio | ✅ | 5 stazioni HTTP MP3 con decoder ESP32-audioI2S, volume e stato |
| microSD | ✅ | File browser, info spazio, screenshot BMP, backup config e punteggi |
| Database IR | 🟡 | 307 file .ir, 38 cartelle: 32 marche TV complete, 5 parziali, 3 coperte da Universale |
| Asset icone 32×32 | ✅ | BMP 24 bit generati, con fallback se assenti |
| Test automatici host | ⬜ | Predisposta la cartella `test/`, nessun test incluso |
| Verifica su hardware reale | ⬜ | Compilazione e filesystem verificati; test su device da fare |

## Funzionalità

- **Menu gerarchico** con carosello orizzontale animato, card centrale e anteprime laterali; le viste con molti elementi usano griglie o liste con auto-repeat.
- **Display** 240×135 in landscape, framebuffer 16 bpp (nessun flicker), tema neubrutalista e rotazione configurabile.
- **Batteria**: indicatore nella barra superiore con percentuale, colore per livello e stato di carica.
- **Storage** LittleFS + microSD opzionale (`/sd`), con fallback automatico e file browser.
- **Configurazione** in NVS, export/import JSON (`/config.json`) senza dati sensibili; backup e screenshot su SD.
- **Giochi interrompibili** con `` ` `` (ESC fisico), record persistenti su SD e ripristino della rotazione all'uscita.
- **IR**: NEC/NECext, Samsung32, LG, Sony/SIRC, RC5/RC5X, RC6, JVC, Kaseikyo (Panasonic), RCA e comandi raw; preferiti in NVS, scanner Power per marca e anteprima dei telecomandi.
- **IR UX**: premi `F` in un telecomando per aggiungere/togliere il preferito. Lo scanner invia il tasto Power di ogni file e permette di salvarlo con `F`.
- **IR ricerca**: la voce **Cerca** interroga tutto il database per marca o modello (sottostringa, senza distinzione di maiuscole).
- **IR tasti rapidi**: dentro un telecomando `P` = Power, `M` = Mute, `=`/`+` = Volume +, `-` = Volume −, `C` = Canale +, `X` = Canale −.
- **BLE**: scansione con nome, MAC e RSSI.
- **Wi-Fi**: scanner con dettagli; configurazione guidata della rete e test di connessione.
- **Radar**: modulo passivo (nessuna trasmissione). *Find BLE* insegue un singolo beacon/tag con media mobile dell'RSSI, distanza stimata dal modello log-distanza, gauge, barra a 16 segmenti con Peak Hold e indicatori `WARMER`/`COLDER`/`HOTSPOT`, con bussola e freccia verso la direzione stimata. *Radar BLE* mostra i dispositivi come blip su un PPI con sweep continuo. *Banda 2.4 GHz* misura i canali Wi-Fi 1–13 con sniffer promiscuo e sovrappone i tre canali advertising BLE 37/38/39. *Presenza CSI* usa Channel State Information con calibrazione su ambiente vuoto.
- **Spegnimento automatico**: dopo un tempo configurabile di inattività il dispositivo abbassa la luminosità e, se non arriva un tasto entro 10 secondi, si spegne.
- **Web Radio**: stazioni pubbliche in MP3 su HTTP (`SomaFM Groove Salad/Drone Zone/Lush`, `Radio Paradise`, `FIP`). `;`/`.` regolano il volume, `ENTER` ferma, `` ` `` esce.
- **Rete WiFi**: configurazione in **Impostazioni → Rete WiFi**. Scansiona le reti vicine, fa scegliere la rete dall'elenco, chiede la password e la salva; permette anche l'inserimento manuale dell'SSID, il test di connessione e la cancellazione delle credenziali.

## Hardware

| Voce | Valore |
|---|---|
| MCU | ESP32-S3FN8, 240 MHz, 320 KB SRAM |
| Flash | 8 MB (partizione app 4 MB + LittleFS 3,9 MB) |
| Display | 1.14" 240×135 |
| Tastiera | 56 tasti (frecce: `;` `.` `,` `/`) |
| IR TX | GPIO 44 |
| microSD | CS 12, MOSI 14, SCK 40, MISO 39 |
| USB | USB-C nativo (CDC) |

## Build da sorgente

Requisiti: Python 3.9+ e PlatformIO. Su Windows:

```powershell
python -m pip install --upgrade platformio
cd cardputer-firmware-public
python -m platformio pkg install
python -m platformio run                 # firmware
python -m platformio run -t buildfs      # immagine LittleFS (icone + database IR)
```

Output:

- `.pio/build/cardputer-adv/firmware.bin`
- `.pio/build/cardputer-adv/littlefs.bin`
- `.pio/build/cardputer-adv/bootloader.bin`
- `.pio/build/cardputer-adv/partitions.bin`

### Flag dei moduli

Ogni modulo si esclude dalla build con il proprio flag (in `platformio.ini` o in `build_flags`):

| Flag | Default | Effetto quando vale 0 |
|---|---|---|
| `ENABLE_MODULE_RADAR` | 1 | esclude `src/modules/radar/` (Finder BLE, PPI, banda, CSI) |
| `ENABLE_MODULE_RADIO` | 1 | esclude `src/modules/radio/` (web radio) |
| `ENABLE_RADAR_CSI` | 1 | compila il radar ma toglie la sola voce "Presenza CSI" |

### Upload

```powershell
python -m platformio run -t upload
python -m platformio run -t uploadfs
```

Se la porta non viene rilevata, specificarla:

```powershell
python -m platformio run -t upload --upload-port COM5
```

> **Dopo ogni upload serve un ciclo di alimentazione.** La USB della Cardputer è quella *nativa* dell'ESP32-S3: qualunque reset comandato da esptool fa ripartire il chip in download mode e lo schermo resta nero. Staccare e riattaccare il cavo USB avvia il firmware.

In caso di problemi, tenere premuto il tasto di boot (G0) e reinserire il cavo USB.

## Flash pronto all'uso (senza toolchain)

La cartella `flasher/` contiene:

- `flash.ps1` / `flash.bat`: installa `esptool` se manca e scrive i binari precompilati.
- `bin/`: bootloader, partizioni, firmware e LittleFS pronti.
- `web/`: flasher browser **ESP Web Tools** (Chrome/Edge desktop).

Il flasher è pubblicato su GitHub Pages: <https://lore11-p.github.io/cardputer-firmware-public/>.
La pagina installa la build pubblica leggendo i binari da questo repository.

Uso rapido:

```powershell
cd flasher
.\flash.bat              # rileva la porta automaticamente
.\flash.bat -Port COM5    # porta esplicita
.\flash.bat -Erase        # cancella tutta la flash prima di scrivere
```

Per il flasher web usa il sito pubblicato: <https://lore11-p.github.io/cardputer-firmware-public/>.
In alternativa apri un server locale nella cartella `flasher/web` e visita `http://localhost:8000`.

```powershell
cd flasher\web
python -m http.server 8000
```

Poi collega la Cardputer via USB e premi **Installa firmware**. Dettagli in `flasher/README.md`.

## Comandi da tastiera

| Tasto | Funzione |
|---|---|
| `;` / `.` | Su / giù (nei menu a carosello scorrono anche le card) |
| `,` / `/` | Sinistra / destra (carosello, griglia, giochi e FPS) |
| ENTER | Seleziona / conferma / spara / fai volare Flappy |
| `` ` `` (ESC fisico) | Indietro / esci dal gioco |
| SPAZIO | Drop Tetris, lancio Breakout, sparo Doom, volo Flappy |
| `F` | Aggiungi/rimuovi il telecomando dai preferiti |
| `P` `M` `=`/`+` `-` `C` `X` | Dentro un telecomando: Power, Mute, Volume +, Volume −, Canale +, Canale − |
| Frecce | Ripeti la sequenza in Simon |
| `w` `a` `s` `d` | Movimento alternativo in Doom Lite |

## Struttura del progetto

```
cardputer-firmware-public/
├── platformio.ini
├── partitions.csv
├── include/                  # header core
├── src/
│   ├── main.cpp
│   ├── core/                 # display, menu, navigazione, UI
│   ├── utils/                # storage, configurazione, rete, input testuale
│   └── modules/
│       ├── ble/              # scanner
│       ├── wifi/             # scanner e configurazione rete
│       ├── radar/            # finder BLE, PPI, banda 2.4 GHz, presenza CSI
│       ├── radio/            # web radio MP3 via ESP32-audioI2S
│       ├── ir/               # telecomandi, preferiti, scanner, TV-B-Gone
│       ├── games/            # Tetris, Breakout, Doom, Snake, Pong, 2048, Flappy, Simon, Minesweeper, Connect 4
│       ├── sd/               # file browser, screenshot, backup
│       └── settings/         # impostazioni e info
├── data/
│   ├── icons/                # icone BMP 32x32
│   └── ir/<marca>/           # 307 file .ir, 38 cartelle
├── flasher/                  # script + binari + flasher web
└── test/                     # predisposto per test host
```

## Configurazione

Le impostazioni sono salvate in NVS nel namespace `cardputer-fw`:

- luminosità (10–255), rotazione (0–3);
- maschera moduli attivi (runtime);
- intervallo e durata delle funzioni radio, ripetizioni IR, ritardo TV-B-Gone;
- spegnimento automatico in secondi (`sleep_timeout_sec`: 0 = disattivato, 30, 60, 120, 300, 600, 900, 1800), default 120.

Le credenziali Wi-Fi non stanno in `config.json`: sono salvate nel namespace NVS `cpfw-net` dal solo `utils/WifiSetup.cpp`, e si gestiscono da **Impostazioni → Rete WiFi**. Si cancellano con "Dimentica rete". Restano in chiaro nella NVS: è l'unico modo per riconnettersi senza reinserire la password a ogni avvio.

Export/import: **Impostazioni → Esporta/Importa config** (`/config.json`). Il JSON non contiene credenziali di rete.

## Formato database IR

I file `.ir` seguono il formato Flipper Zero:

```
Filetype: IR signals file
Version: 1
#
name: Power
type: parsed
protocol: Samsung32
address: 07 00 00 00
command: 02 00 00 00
```

Protocolli parsed supportati: `NEC`, `NECext`, `Samsung32`, `LG`, `Sony`/`SIRC12/15/20`, `RC5`, `RC5X`, `RC6`, `JVC`, `Kaseikyo` (Panasonic) e `RCA`. I comandi `type: raw` con `frequency` e `data` in microsecondi sono inviati con `sendRaw`. I file sono letti da LittleFS e, se assenti, da `/sd/ir/...`. I comandi con protocolli non riconosciuti sono marcati `[n/d]`; dettagli e provenienza sono in `data/ir/SOURCES.txt`. LittleFS è dimensionata a 3,9 MB e i nomi dei file devono restare entro 32 caratteri.

## Limiti noti

- Il database IR non contiene codici TV dedicati per Skyworth, OnePlus e Realme: usa la cartella `Universale`. ASUS, Huawei, Majestic, Trevi e Inno-Hit hanno solo codici parziali o per altri dispositivi.
- Il modulo IR dispone solo del trasmettitore: `ENABLE_IR_LEARN=1` richiede un ricevitore esterno e codice dedicato.
- Doom Lite è un raycaster senza texture, con 3 mappe e nemici semplici.
- Nessuna PSRAM: il framebuffer e le risorse sono dimensionati per la SRAM disponibile.
- Radar: la radio è una sola, quindi Wi-Fi e BLE non possono ascoltare insieme. La vista banda alterna le due finestre (TDM) e mostra il riparto del tempo.
- Radar: NimBLE non espone il canale di ricezione di un advertising, quindi l'attività dei canali BLE 37/38/39 è stimata dal rate aggregato; i canali Wi-Fi sono invece misurati davvero.
- Radar: il "single target" ricava un bearing con IMU e schermatura del corpo. Qui `WARMER`/`COLDER` indicano la variazione di distanza stimata, non la direzione.
- Radar CSI (`ENABLE_RADAR_CSI`): senza un access point che generi traffico arrivano solo i beacon degli AP vicini; la sensibilità è inferiore a un sistema con traffico dedicato e va ricalibrata a ogni cambio canale.
- Web Radio: solo stream **HTTP in MP3/AAC** (niente HTTPS) e una stazione alla volta. Durante l'ascolto il bus I2S è del decoder, quindi i beep del radar non suonano; il decoder consuma ~40 KB di heap.
- Il progetto è compilato e verificato a livello di build; il collaudo su hardware reale è a carico dell'utente.

## Licenza

Il codice del firmware è distribuito con licenza **MIT** (vedi `LICENSE`).
Il database IR proviene da Flipper-IRDB ed è rilasciato sotto **CC0 1.0**;
l'attribuzione è riportata in `data/ir/SOURCES.txt`. Le librerie di terze
parti mantengono le rispettive licenze.

## Crediti

M5Stack M5Cardputer/M5Unified/M5GFX, h2zero NimBLE-Arduino, crankyoldgit IRremoteESP8266, ESPAsyncWebServer/AsyncTCP (esphome), bblanchon ArduinoJson, Espressif ESP-IDF/Arduino core. Database IR: Flipper-IRDB di Lucaslhm, licenza CC0 1.0; dettagli in `data/ir/SOURCES.txt`. Ispirazione per le funzioni IR: [Ultimate-Remote di geo-tp](https://github.com/geo-tp/Ultimate-Remote), progetto separato per Cardputer; qui il codice è indipendente. Rispetta le rispettive licenze.

Uso previsto: studio, prototipazione e test su dispositivi propri.
