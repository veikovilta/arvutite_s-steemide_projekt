# Arvutisüsteemide projekt

See projekt on mõeldud kasutamiseks viite mõõtmiseks kasutades raspberry pi4B-d. 

Projekt kasutab I2C-põhist OLED-ekraani juhtimist, GPIO pin’ide haldust, multithreading'ut ning sensori põhist andmetöötlust. 
Rakendus võimaldab töötada kahes režiimis: kas saatjana või vastuvõtjana.

---

## Autorid
- **Veiko Vilta**
- **Kaur Andro Orgusaar**

---

## Omadused
- **I2C suhtlus:** OLED-ekraani algatamine ja kasutamine tagasiside andmiseks.
- **Multithreading:** Kasutab lõime, et hallata nupuvajutusi ja muid asünkroonseid ülesandeid.
- **Chrony integratsioon:** Süsteemi aja sünkroniseerimine täpse ajastuse saavutamiseks.
- **GPIO haldus:** LED-ide ja sensorite seadistamine ning haldamine.
- **Töörežiimid:**
  - **Saatja:** Vilgutab LED-i täpsetel ajavahemikel.
  - **Vastuvõtja:** Tuvastab ja töötleb LED-i vilkumisi ning arvutab keskmise viivituse.

---

## Koodi põhijooned
### Sõltuvused
Projekt vajab järgmisi teeke ja raamatukogusid:
- `pthread.h` lõimede haldamiseks.
- `unistd.h` täpsete viivituste ja süsteemioperatsioonide jaoks.
- `gpiod.h` GPIO pin’ide haldamiseks.
- `stdbool.h` loogika jaoks.
- Kohandatud päisfailid:
  - `HelperFunctions.h` - Utiliidid, nagu ajatemplite ja puhvri haldamine.
  - `display.h` - OLED-ekraani juhtimine.
  - `Sensor.h` - Sensorite andmete haldus.
  - `LedBlink.h` - LED-i vilkumise funktsioonid.
  - `State.h` - Rakenduse olekute haldamine.
  - `Files.h` - Failioperatsioonid, näiteks logimine.
  - `Main.h` - Põhimeetodid ja definitsioonid.

### Põhifunktsionaalsus
#### Initsialiseerimine
- Chrony teenuse käivitamine aja sünkroniseerimiseks.
- I2C suhtluse algatamine OLED-ekraani jaoks.

#### Töörežiimid
- **Saatja:**
  - LED-i täpne vilgutamine.
  - Järgmise minuti ootamine täpsuse säilitamiseks.
  - Alguse ja lõpu ajastuse logimine.
- **Vastuvõtja:**
  - LED-i vilkumiste tuvastamine.
  - Keskmise viivituse arvutamine ja ekraanil kuvamine.
  - Tulemuste logimine analüüsimiseks.

#### Lõpetamine
- Resursside (lõimed, GPIO-d, OLED-ekraan) korrektne vabastamine.
- Logide kirjutamine faili edasiseks analüüsiks.

---

