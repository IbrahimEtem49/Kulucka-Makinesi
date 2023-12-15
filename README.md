# Kulucka-Makinesi

Bu projede kullanılan malzemeler;
- DHT22 Nem ve Sıcaklık Sensörü
- DS18B20 Sıcaklık Sensörü
- 16x2 I2C LCD Ekran
- 4 adet Push-buton
- Kırmızı ve Yeşil LED
- Buzzer
- 2 adet fan, büyüklükleri ve çalışma voltajları opsiyonel
- 2'li Röle Modülü
- MG996R Servo Motor
- Eski bir uydu alıcısından sökülmüş besleme devresi (3.3V, 6V ve 12V çıkışları bulumakta)
  - L7805CV Voltaj Regülatörü
    - Regülatör ile kullanılmak üzere 0.22µF ve 0.1µF kondansatör

Şu anda projede 2 temel sorun var:
- Bunlardan ilki enerji kesilirse değişkenlerdeki değerler gidiyor. (EEPROM'da kayıtlı olanlar hariç)
Özellikle millis değerinin sıfırlanması işleri zora sokabilir. Bunun için aklımda 2 çözüm var. Birinci
ve bana en mantıklı geleni, belirli aralıklarla (6 saatte bir uygun olur sanırım) millis değerini EEPROM'a
yedeklemek. İkincisi de sadece geçen gün sayısını, her gün geçişinde EEPROM'a kaydetmek ve 22. güne
geçildiği anda geçen gün sayısını tutan değişkeni sıfırlamak. Bunu yakın bir zamanda çözmeyi planlıyorum.

- İkinci sorun ise tamamen benden kaynaklı. Kullandığım güç kaynağının çıkış gücü oldukça düşük. 
Bu yüzden güç kaynağına bağlı birden fazla eleman aynı anda çalıştığında rölenin kontakları çok
hızlı bir şekilde açılıp kapanıyor. Güç kaynağımı güncellediğimde eğer bu sorun çözülürse bunu da yine
burada bildiririm.
