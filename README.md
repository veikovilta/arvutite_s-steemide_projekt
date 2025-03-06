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






#### Lisa  Tootmise ja seadistamise juhend
Käesolev projekt on loodud eesmärgiga testida lahendust, mida saaks kasutada hilistuse mõõtmiseks vajalike seadmete tootmisel. Selles peatükis on esitatud ka lühike juhend seadmete valmistamiseks ja seadistamiseks.

## .1  Komponentide hankimine
Tootmise esimeseks sammuks on vajalike komponentide hankimine, mille loetelu on toodud lisas 3. Lisaks on lisas esitatud ka trükkplaadi ja seadme korpuse loomise skeemid ning vajalikud failid. Kõik vastavad failid on kättesaadavad GitHubi repositooriumis, mille link on toodud lisas 2.

## .2  Trükkplaadi tootmine ja jootmine
Trükkplaadi valmistamiseks on saadaval Gerber-failid (pakitud .rar formaadis), mida saab edastada trükkplaadi tootmise ettevõttele, kes valmistab plaadi vastavalt spetsifikatsioonidele.
Kui trükkplaat on valmis, tuleb sellele joota komponendid vastavalt PCB skeemis toodud juhistele.

## .3  Seadme kokkupanek
Jootmise järgselt tuleb kokku monteerida järgmised komponendid:
•	Trükkplaat
•	PoE-HAT moodul
•	Raspberry Pi 4B
Ühendamiseks kasutatakse vastavaid ühenduslülisid ja pin-päiseid. Allpool on esitatud joonis (joonis X), mis illustreerib ühenduste korrektset paigutust.



Kui seadme riistvara on kokkupandud, saab liikuda tarkvara paigaldamise juurde.

## .4 Tarkvara paigaldamine ja seadistamine
Seadme tarkvara toimimiseks on vaja paigaldada Raspberry Pi operatsioonisüsteem, laadida alla programmi failid ning seadistada Chrony ajasünkroonimine.

## .4.1  Operatsioonisüsteemi paigaldamine
Operatsioonisüsteemi paigaldamiseks tuleb kasutada Raspberry Pi Imager-it. Kasutaja saab valida kas graafilise liidesega või ilma liideseta operatsioonisüsteemi – viimane võib olla veidi kiirem.

## .4.2  Chrony sünkroniseerimise seadistamine
Pärast operatsioonisüsteemi paigaldamist tuleb käivitada järgmised käsud:
sudo apt update
sudo apt install chrony -y
sudo nano /etc/chrony/chrony.conf
Seejärel tuleb chrony.conf faili sisu asendada GitHubis olevaga. Pärast faili muutmist tuleb käivitada:
sudo systemctl restart chronyd
sudo systemctl enable chronyd
Kontrollimaks, kas Chrony töötab korrektselt, võib kasutada käske:
systemctl status chronyd
chronyc tracking
Kui väljundis kuvatakse sünkroniseeritud serverid ja vastav ajakõrvalekalle, võib eeldada, et Chrony töötab korrektselt.

(Siia võib lisada illustratsiooni või näidispildi korrektse väljundi kohta.)

## .5  Programmi paigaldamine ja automaatkäivituse seadistamine
Viimase sammuna tuleb alla laadida rakenduse failid GitHubist ning seadistada need automaatselt käivituma igal süsteemi käivitamisel.
sudo nano /etc/rc.local
Faili lõppu tuleb lisada programmi asukoht, näiteks:
/home/pi/Programm &
Pärast muudatuste salvestamist tuleb Raspberry Pi taaskäivitada:
sudo reboot
Seejärel saab testida, kas lahendus töötab ootuspäraselt.


