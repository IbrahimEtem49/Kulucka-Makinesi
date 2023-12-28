#include <EEPROM.h>

#include <Servo.h>
Servo myServo;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <OneWire.h>
#include <DallasTemperature.h>
#define DSPIN A0
#define ONE_WIRE_BUS DSPIN
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#include "DHT.h"
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include <LcdMenu.h>

void anaMenu();
void sicaklikAyar();
void nemAyar();
void cevirmeAyar();
void gunAyar();
void eepromKontrol();
void sifirlama();

MAIN_MENU(
  ITEM_COMMAND("Ana Menu", anaMenu),
  ITEM_COMMAND("Sicaklik Ayari", sicaklikAyar),
  ITEM_COMMAND("Nem Ayari", nemAyar),
  ITEM_COMMAND("Cevirme Ayari", cevirmeAyar),
  ITEM_COMMAND("Kulucka Suresi", gunAyar),
  ITEM_COMMAND("EEPROM Kontrol", eepromKontrol),
  ITEM_COMMAND("Reset", sifirlama)
);

LcdMenu menu(2, 16);

const uint8_t bMenu = 2, bGeri = 3, bYukari = 4, bAsagi = 5, ledG = 6, ledR = 7, buzzer = 8, ampul = 9, nemNozulu = 10;
float sicaklik, nem;

const uint16_t saniye = 1000, dakika = 60000;
const uint32_t saat = 3600000, gun = 86400000, gun21 = 1814400000;
uint32_t sonDonus = 0;
int sayac = 0;

uint32_t sonSicaklikOkuma = 0;
uint32_t sonNemOkuma = 0;
uint32_t sonYedek = 0;
uint32_t yedek;

float sicaklikDegeri = 37.5, nemDegeri = 50.0;
uint8_t dondurme = 6, kuluckaSuresi = 21;

uint8_t kayitSicaklik = 100, kayitNem = 120, kayitDonus = 140, kayitKuluckasuresi = 160, kayitYedek = 180;

float eepromNem, eepromSicaklik;
uint8_t eepromDonus, eepromKuluckasuresi;
uint32_t eepromYedek;

bool buzzerCalisti = false;

int kalanGun;

#define BUTON_OKUMA(btn, islem) \
  if (digitalRead(btn)) { \
    while (digitalRead(btn)); \
    menu.islem(); \
  }

void setup() {
  EEPROM.begin();
  lcd.init();
  lcd.backlight();
  myServo.attach(11);
  sensors.begin();
  dht.begin();
  pinMode(bMenu, INPUT); pinMode(bYukari, INPUT); pinMode(bAsagi, INPUT); pinMode(bGeri, INPUT);
  pinMode(ledG, OUTPUT); pinMode(ledR, OUTPUT); pinMode(buzzer, OUTPUT); pinMode(ampul, OUTPUT); pinMode(nemNozulu, OUTPUT);
  digitalWrite(ampul, 1);
  digitalWrite(nemNozulu, 0);

  menu.setupLcdWithMenu(0x27, mainMenu);
}

void loop() {
  eepromNem = EEPROM.get(kayitNem, nemDegeri);
  eepromSicaklik = EEPROM.get(kayitSicaklik, sicaklikDegeri);
  eepromDonus = EEPROM.get(kayitDonus, dondurme);
  eepromKuluckasuresi = EEPROM.get(kayitKuluckasuresi, kuluckaSuresi);
  eepromYedek = EEPROM.get(kayitYedek, yedek);

  sensors.setResolution(12);
  sicaklikOkuma();
  nemOkuma();

  yedekleme();
  donus(dondurme);
  sicaklikUyari();
  nemUyari();
  uyari();

  kalanGun = (gun + yedek)/gun;
  if (kalanGun > eepromKuluckasuresi){
    yedek = 0;
    kalanGun = 0;
  } 

  BUTON_OKUMA(bAsagi, down)
  BUTON_OKUMA(bYukari, up)
  BUTON_OKUMA(bMenu, enter)
  BUTON_OKUMA(bGeri, back)
}

void nemOkuma(){
  if (millis() - sonNemOkuma > 3*saniye) {
    nem = dht.readHumidity();
    sonNemOkuma = millis();
  }
}

void sicaklikOkuma(){
  if (millis() - sonSicaklikOkuma > 3*saniye) {
    sensors.requestTemperatures();
    sicaklik = sensors.getTempCByIndex(0);
    sonSicaklikOkuma = millis();
  }
}

void donus(uint32_t donusAralik) {
  if (millis() - sonDonus > donusAralik*saat) {
    myServo.write(30);
    digitalWrite(ledR, 1);
    delay(1500);
    myServo.write(90);
    digitalWrite(ledR, 0);
    sonDonus = millis();
    sayac += 1;
  }
}

bool sicaklikUyari() {
  if (sicaklik < eepromSicaklik - 1) {
    digitalWrite(ampul, 0);
    return 1;
  } else {
    digitalWrite(ampul, 1);
    return 0;
  }
}

bool nemUyari() {
  if (nem < eepromNem - 5) {
    digitalWrite(nemNozulu, 1);
    return 1;
  } else {
    digitalWrite(nemNozulu, 0);
    return 0;
  }
}

void uyari() {
  bool sUyari = sicaklikUyari(), nUyari = nemUyari();
  if ((sUyari == 1) || (nUyari == 1)) {
    digitalWrite(ledR, 1); digitalWrite(ledG, 0);
    if (!buzzerCalisti) { 
      tone(buzzer, 1000); 
      delay(2000);
      noTone(buzzer);
      buzzerCalisti = true; 
    }
  } else {
    digitalWrite(ledR, 0); digitalWrite(ledG, 1);
    buzzerCalisti = false; 
  }
}

void anaMenu() {
  lcd.clear();
  while (true) {
    sicaklikOkuma();
    nemOkuma();
    yedekleme();
    donus(dondurme);
    sicaklikUyari();
    nemUyari();
    uyari();
    lcd.setCursor(0, 0);
    lcd.print("Gun:"); lcd.print(kalanGun); lcd.print("/"); lcd.print(eepromKuluckasuresi); lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(sicaklik); lcd.print((char)223); lcd.print("C|"); lcd.print(nem); lcd.print("%RH");
    if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      break;
    }
  }
}

void sicaklikAyar() {
  eepromSicaklik = EEPROM.get(kayitSicaklik, sicaklikDegeri);
  lcd.clear();

  while (true) {
    lcd.home();
    lcd.print("Eski deger:"); lcd.print(eepromSicaklik); lcd.print((char)223); lcd.print("C");
    lcd.setCursor(0,1); 
    lcd.print("Yeni deger:"); lcd.print(sicaklikDegeri); lcd.print((char)223); lcd.print("C");

    if (digitalRead(bYukari)) {
      sicaklikDegeri = constrain(sicaklikDegeri + 0.1, 35.0, 40.0);
      delay(100);
    } else if (digitalRead(bAsagi)) {
      sicaklikDegeri = constrain(sicaklikDegeri - 0.1, 35.0, 40.0);
      delay(100);
    } else if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      lcd.clear();
      EEPROM.put(kayitSicaklik, sicaklikDegeri);
      lcd.print("Islem basarili!");

      delay(1000);
      break;
    }
  }
}

void nemAyar() {
  eepromNem = EEPROM.get(kayitNem, nemDegeri);
  lcd.clear();

  while (true) {
    lcd.home();
    lcd.print("Eski deger:"); lcd.print(eepromNem); lcd.print("%RH");
    lcd.setCursor(0,1); 
    lcd.print("Yeni deger:"); lcd.print(nemDegeri); lcd.print("%RH");

    if (digitalRead(bYukari)) {
      nemDegeri = constrain(nemDegeri + 0.1, 40.0, 80.0);
      delay(100);
    } else if (digitalRead(bAsagi)) {
      nemDegeri = constrain(nemDegeri - 0.1, 40.0, 80.0);
      delay(100);
    } else if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      lcd.clear();
      EEPROM.put(kayitNem, nemDegeri);
      lcd.print("Islem Basarili!");

      delay(1000);
      break;
    }
  }
}

void cevirmeAyar() {
  eepromDonus = EEPROM.get(kayitDonus, dondurme);
  lcd.clear();

  while (true) {
    lcd.home();
    lcd.print("Eski sure:"); lcd.print(eepromDonus); lcd.print(" saat");
    lcd.setCursor(0,1); 
    lcd.print("Yeni sure:"); lcd.print(dondurme); lcd.print(" saat");

    if (digitalRead(bYukari)) {
      while(digitalRead(bYukari));
      dondurme = constrain(dondurme + 1, 2, 8);
      delay(50);
    } else if (digitalRead(bAsagi)) {
      while(digitalRead(bAsagi));
      dondurme = constrain(dondurme - 1, 2, 8);
      delay(50);
    } else if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      lcd.clear();
      EEPROM.put(kayitDonus, dondurme);
      lcd.print("Islem Basarili!");

      delay(1000);
      break;
    }
  }
}

void gunAyar() {
  eepromKuluckasuresi = EEPROM.get(kayitKuluckasuresi, kuluckaSuresi);
  lcd.clear();

  while(true){
    lcd.home();
    lcd.print("Eski sure:"); lcd.print(eepromKuluckasuresi); lcd.print(" gun");
    lcd.setCursor(0,1); 
    lcd.print("Yeni sure:"); lcd.print(kuluckaSuresi); lcd.print(" gun");

    if (digitalRead(bYukari)) {
      while(digitalRead(bYukari));
      kuluckaSuresi = constrain(kuluckaSuresi + 1, 16, 30);
      delay(50);
    } else if (digitalRead(bAsagi)) {
      while(digitalRead(bAsagi));
      kuluckaSuresi = constrain(kuluckaSuresi - 1, 16, 30);
      delay(50);
    } else if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      lcd.clear();
      EEPROM.put(kayitKuluckasuresi, kuluckaSuresi);
      lcd.print("Islem Basarili!");

      delay(1000);
      break;
    }
  }
}

void eepromKontrol() {
  lcd.clear();
  
  while (true) {
    lcd.home();
    lcd.print("Sic:"); lcd.print(eepromSicaklik); lcd.print("|Gun:"); lcd.print(eepromKuluckasuresi);
    lcd.setCursor(0,1);
    lcd.print("Nem:"); lcd.print(eepromNem); lcd.print("|Don:"); lcd.print(eepromDonus);
    if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      break;
    }
  }
}

void sifirlama() {
  lcd.clear();

  while(true){
    lcd.home();
    lcd.print("Onay icin "); lcd.print("(Up)");
    lcd.setCursor(0,1);
    lcd.print("Red icin  "); lcd.print("(Down)");

    if (digitalRead(bYukari)) {
      while(digitalRead(bYukari));
      EEPROM.put(kayitNem, 50.00);
      EEPROM.put(kayitSicaklik, 37.50);
      EEPROM.put(kayitDonus, 6);
      EEPROM.put(kayitKuluckasuresi, 21);

      lcd.clear(); lcd.home();
      lcd.print("Islem basarili!");

      delay(1000);
      break;
    } else if (digitalRead(bAsagi)) {
      while(digitalRead(bAsagi));

      lcd.clear(); lcd.home();
      lcd.print("Islem iptal");
      lcd.setCursor(0,1);
      lcd.print("edildi!");
      
      delay(1000);
      break;
    } else if (digitalRead(bMenu)) {
      while(digitalRead(bMenu));
      break;
    }
  }
}

void yedekleme(){
  if (millis() - sonYedek > 10*dakika) {
    if(millis() < yedek){
      yedek = yedek + millis();
    } else {
      yedek = millis();
    }
    EEPROM.put(kayitYedek, yedek);
    sonYedek = millis();
  }
}
