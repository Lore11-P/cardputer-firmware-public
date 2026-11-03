# Changelog

Tutte le modifiche rilevanti del progetto. Formato basato su Keep a Changelog.

## [Unreleased]

### Aggiunto

- Indicatore batteria nella barra superiore: percentuale, colore per livello e stato di carica, visibile in tutte le schermate con header.
- Sito web del flasher su GitHub Pages, in stile neubrutalista, con installazione via Web Serial e binari letti dal repository.
- Modulo **Radar** (`src/modules/radar/`, `ENABLE_MODULE_RADAR`): modulo passivo con finder BLE hot/cold, radar PPI, vista banda 2.4 GHz con TDM Wi-Fi/BLE e presenza CSI sperimentale (`ENABLE_RADAR_CSI`).
- Credenziali Wi-Fi in namespace NVS dedicato `cpfw-net`, cancellabili da menu.
- Nuova voce **Impostazioni > Rete WiFi**: scanner delle reti, scelta dall'elenco, inserimento della password, salvataggio in NVS, test di connessione, inserimento manuale per reti nascoste e cancellazione delle credenziali.
- `utils/WifiSetup` (`include/WifiSetup.h` + `src/utils/WifiSetup.cpp`): unico punto che salva la password Wi-Fi, usato da Impostazioni > Rete WiFi.
- `utils/TextInput` (`include/TextInput.h` + `src/utils/TextInput.cpp`): editor di testo a schermo condiviso, usato dalla configurazione rete e dalla ricerca IR.
- IR: voce **Cerca** che filtra per marca o modello su tutto il database `.ir`.
- IR: **tasti rapidi** nel telecomando — `P` Power, `M` Mute, `=`/`+` Volume +, `-` Volume −, `C` Canale +, `X` Canale −.
- IR: **Riprendi** ultimo telecomando aperto come prima voce del menu, salvato in NVS.
- **Spegnimento automatico** per inattività (`include/PowerTimer.h` + `src/core/PowerTimer.cpp`): timeout configurabile, preavviso di 10 secondi annullabile con qualsiasi tasto.
- Menu principale, menu Giochi e menu IR a carosello orizzontale animato.
- Quattro nuovi giochi: Flappy, Simon, Minesweeper e Connect 4.
- **Modulo Radio** (`src/modules/radio/`, `ENABLE_MODULE_RADIO`): web radio MP3/AAC su HTTP con decoder `esphome/ESP32-audioI2S`.
- Bussola a heading relativo (`include/Compass.h` + `src/core/Compass.cpp`): integrazione del giroscopio BMI270.
- Radar: freccia direzionale nel finder BLE, tic Geiger legato alla distanza e sensibilità configurabile (livelli 1–5, salvata in NVS).

### Modificato

- `SettingsMenu`: elenco dei moduli attivi scrollabile con indicatore di posizione.
- Menu principale e Giochi a carosello invece della griglia 3x2.
- README e CHANGELOG aggiornati con comandi, giochi, funzioni IR, modulo Radar, flag e limiti noti.

### Corretto

- Crash entrando in Infrarossi e Giochi causato da stack overflow del task dei menu. Buffer di `drawIcon` resi statici e `ARDUINO_LOOP_STACK_SIZE=16384`.
- *Info dispositivo* mostra lo stack residuo minimo.

## [1.0.0] - 2026-10-31

### Aggiunto

- Struttura PlatformIO per M5Stack Cardputer ADV (ESP32-S3FN8, 8 MB, no PSRAM).
- Menu gerarchico con icone BMP 32x32, fallback vettoriale, breadcrumb e scrollbar.
- Framebuffer 16 bpp con rotazione configurabile e gestione display senza flicker.
- Storage LittleFS + microSD con fallback automatico e logger seriale.
- Configurazione persistente NVS, maschera moduli runtime, export/import JSON.
- Modulo Impostazioni: luminosità, rotazione, info chip/batteria/heap, moduli, reset.
- Giochi: Tetris, Breakout, Doom Lite, Snake, Pong e 2048 con record persistenti su microSD.
- Modulo microSD: file browser, info spazio, screenshot BMP, backup di configurazione e punteggi.
- Modulo IR: parser Flipper `.ir`, NEC/NECext, Samsung32, LG, Sony/SIRC, RC5/RC5X, RC6, JVC, Kaseikyo (Panasonic), RCA, raw e TV-B-Gone.
- Modulo BLE: scanner con deduplica e dettagli.
- Modulo Wi-Fi: scanner asincrono con dettagli e configurazione rete.
- Database IR con 307 file `.ir` e 38 cartelle.
- Flasher PowerShell con installazione automatica di esptool e flasher web ESP Web Tools.
- README con build, flash, comandi, struttura, limiti e licenza.

### Corretto

- LittleFS portata a 3,9 MB e rinominati due file oltre 32 caratteri.
- I comandi parsed sono marcati come supportati solo se il protocollo è riconosciuto.
- Indirizzi e comandi Flipper a 4 byte vengono letti in little-endian.
- Corretto il puntatore al font in 2048 che impediva la compilazione con M5GFX.

### Licenza

- Aggiunta licenza MIT per il firmware; database IR CC0 1.0 con attribuzione in `data/ir/SOURCES.txt`.

### Limitato / noto

- IR: nessun codice TV dedicato per Skyworth, OnePlus e Realme (usare Universale); ASUS, Huawei, Majestic, Trevi e Inno-Hit hanno copertura parziale.
- IR learn richiede un ricevitore esterno (non incluso).
- Doom Lite senza texture e con IA nemica semplice.
- I record dei giochi vengono salvati su microSD; senza SD non sono persistenti.
- Verificare pin SD, IR e comportamento radio sul dispositivo reale.

## Edizione pubblica

Questa edizione contiene solo i moduli pubblici (scanner, radar passivo, radio, IR, giochi, SD e impostazioni). I blocchi sperimentali non pubblici sono stati rimossi dal sorgente e dal flasher.
