# Zadanie 5 – Čítanie senzorov HTS221 a LPS25HB cez I2C a odosielanie dát cez USART2

**Meno:** Dominik Ďurmek
**AIS ID:** 126716
**Predmet:** VRS – Vnorené riadiace systémy
**Cvičenie:** štvrtok 10:00

**Poznámka:** Tento README súbor bol vygenerovaný pomocou AI (Claude od Anthropic) a upravený podľa môjho riešenia.
Pri riešení zadania som využil AI asistenta (Claude) na kontrolu kódu, kontrolu registrov podľa datasheetov a na zostavenie tohto README.

## Čo program robí

Program beží na doske Nucleo (STM32F303K8) so senzorovou doskou X-NUCLEO-IKS01A1. Cez I2C1 číta teplotu a vlhkosť zo senzora HTS221 a tlak zo senzora LPS25HB. Z tlaku počíta relatívnu výšku od zeme a všetky hodnoty posiela cez USART2 do PC vo formáte CSV. Projekt je vytvorený v CubeMX s využitím LL knižnice.

1. **I2C komunikácia** – funkcie `i2c_master_read` a `i2c_master_write` podporujú prenos jedného aj viacerých bajtov. Čítanie používa repeated start a prijaté bajty ukladá prerušenie od I2C (RXNE) priamo do buffera. Funkcie majú ošetrený NACK a timeout, takže ak senzor neodpovedá, program sa nezasekne.
2. **Knižnice senzorov** – drivery `HTS221` a `LPS25HB` nepoužívajú I2C funkcie priamo. Funkcie z `i2c.c` sa im odovzdajú ako callbacky (`hts221_register_callbacks`, `lps25hb_register_callbacks`), vďaka čomu je konfigurácia MCU oddelená od konfigurácie senzorov.
3. **HTS221** – inicializácia overí register `WHO_AM_I` (0xBC), zapne senzor, nastaví block data update a frekvenciu merania 12,5 Hz. Kalibračné registre (0x30 – 0x3F) sa načítajú naraz jedným multi-byte čítaním. Teplota aj vlhkosť sa z nameraných hodnôt vypočítajú lineárnou interpoláciou podľa datasheetu, vlhkosť sa orezáva na rozsah 0 – 100 %.
4. **LPS25HB** – inicializácia overí register `WHO_AM_I` (0xBD) a skúša obe možné adresy senzora (podľa zapojenia pinu SA0, 0xBA a 0xB8). Následne zapne senzor, nastaví block data update a frekvenciu 12,5 Hz. Tlak (24 bitov) sa číta naraz troma bajtmi a prepočíta sa na hPa (delením 4096).
5. **Výpočet relatívnej výšky** – samostatná funkcia `lps25hb_calc_altitude` počíta výšku z barometrického vzorca `h = 44330 · (1 − (p / p0)^(1/5,255))`. Referenčný tlak `p0` sa určí ako priemer z 8 meraní krátko po spustení, takže výška je vzhľadom na miesto, kde sa doska zapla.
6. **Odosielanie dát** – každých približne 100 ms sa pošle jeden riadok vo formáte `teplota,vlhkosť,tlak,výška`, napr. `23.4,45,1013.25,0.12`.

Formát odosielaných hodnôt:

| Veličina | Jednotka | Formát |
|---|---|---|
| teplota | °C | `xx.x` |
| relatívna vlhkosť | % | `xx` |
| tlak vzduchu | hPa | `xxxx.xx` |
| relatívna výška od zeme | m | `xx.xx` |

## Kde je čo v kóde

* `Src/i2c.c` – nastavenie I2C1 a funkcie `i2c_master_read`, `i2c_master_write`, `i2c_rx_irq_handler` (vrátane ošetrenia NACK a timeoutu)
* `HTS221/hts221.h`, `HTS221/hts221.c` – makrá registrov, inicializácia, čítanie teploty a vlhkosti
* `LPS25HB/lps25hb.h`, `LPS25HB/lps25hb.c` – makrá registrov, inicializácia, čítanie tlaku a výpočet výšky
* `Src/usart.c` – nastavenie USART2 a funkcia `usart_send_string`
* `Src/main.c` – registrácia callbackov, inicializácia senzorov, určenie referenčného tlaku a odosielanie CSV vo `while` slučke
* `Src/stm32f3xx_it.c` – obsluha prerušenia `I2C1_EV_IRQHandler`, ktorá volá `i2c_rx_irq_handler`
* `docs/` – datasheety senzorov

## Ako to vyskúšať

1. Nasadiť senzorovú dosku X-NUCLEO-IKS01A1 na Nucleo a nahrať program do dosky.
2. Otvoriť Serial Oscilloscope (alebo iný terminál, napr. PuTTY) na porte ST-Link, rýchlosť 38400, 8 dátových bitov, bez parity, 1 stop bit.
3. Po spustení sa približne 1 sekundu meria referenčný tlak, potom začnú chodiť riadky s hodnotami. V Serial Oscilloscope sa zobrazia 4 kanály (teplota, vlhkosť, tlak, výška).

Ak sa inicializácia niektorého senzora nepodarí (senzor neodpovedá alebo vráti zlú hodnotu `WHO_AM_I`), LED na PB3 rýchlo bliká a program nepokračuje.