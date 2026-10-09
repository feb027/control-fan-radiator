# CR-V Gen 3 Emergency Radiator Fan Controller (Arduino Nano)

Sistem pengontrol darurat kipas radiator (*radiator fan*) untuk Honda CR-V Generasi 3 berbasis **Arduino Nano (ATmega328P)**, layar **OLED 0.91" I2C (128x32)**, modul relay 5V, dan penyadapan sinyal dari sensor **ECT (Engine Coolant Temperature)** bawaan mobil.

---

## 🚀 Fitur Utama

1. **Non-Blocking & Ultra-Responsive**:
   - Loop berjalan tanpa `delay()`, menggunakan penjadwalan waktu berbasis `millis()`.
   - Refresh layar OLED berjalan di 120 ms (bebas flicker dan tidak lag).
   - Input tombol dibaca setiap siklus dengan debouncing otomatis.
2. **Modular & Scalable Architecture**:
   - Terbagi rapi ke dalam modul `EctSensor`, `ButtonHandler`, `RelayController`, `DisplayManager`, dan `Storage`.
3. **Safety & Fail-Safe Protection**:
   - Jika kabel sensor ECT putus (open circuit) atau korslet ke ground (short to GND), fan **otomatis dipaksa ON (Forced ON)** dan layar menampilkan pesan peringatan.
   - Dilengkapi *dwell protection* (jeda minimal 2 detik antar saklar) dan histeresis untuk mencegah relay *chattering* (cetak-cetek).
4. **Kalibrasi & Konfigurasi On-the-Fly (2 Tombol)**:
   - Pengaturan batas atas (Fan ON), batas bawah (Fan OFF), dan offset kalibrasi suhu.
   - Semua setelan tersimpan aman di **EEPROM** dengan verifikasi checksum (tidak hilang saat kunci kontak dimatikan).
5. **Kurva Sensor OEM Honda**:
   - Menggunakan tabel pemetaan (*Look-Up Table*) NTC Honda CR-V dengan interpolasi linier presisi tinggi dan filter *Exponential Moving Average* (EMA).

---

## 🔌 Skema Pinout & Wiring

| Komponen | Pin Arduino Nano | Keterangan |
| :--- | :--- | :--- |
| **Sinyal ECT Mobil** | `A0` | Sadap kabel sinyal ECT (lewatkan filter RC) |
| **OLED 0.91" (SDA)** | `A4` | I2C Data |
| **OLED 0.91" (SCL)** | `A5` | I2C Clock |
| **OLED VCC / GND** | `5V` / `GND` | Catu daya layar |
| **Tombol A (Set / +)**| `D2` | Tombol push-button ke `GND` (Internal Pull-Up) |
| **Tombol B (Next / -)**| `D3` | Tombol push-button ke `GND` (Internal Pull-Up) |
| **Driver Relay 12V** | `D7` | Masuk ke Resistor 1kΩ -> Base Transistor NPN (2N2222) (Active HIGH) |

### Rangkaian Driver Relay 12V (NPN Transistor)
```
               +12V Aki / ACC Mobil
                     |
                     +-------------------+
                     |                   |
               [Pin 86 Koil Relay]       |
                     |             (Katoda / Gelang Putih)
                     |                [ Dioda 1N4007 ]
                     |             (Anoda / Tanpa Gelang)
               [Pin 85 Koil Relay]       |
                     |                   |
                     +-------------------+
                     |
                 Collector (C)
                 +-----------+
Arduino D7 >--[ 1k R ]--| B  2N2222 |
                 +-----------+
                 Emitter (E)
                     |
               GND (Massa Mobil / Arduino)
```

### Rangkaian Filter Sinyal ECT (Wajib untuk Mobil)
```
Kabel Sinyal ECT Mobil ----[ 10k Ohm Resistor ]----+----> Pin A0 Arduino
                                                   |
                                            [ 100nF Kapasitor ]
                                                   |
Ground Mobil / ECU --------------------------------+----> Pin GND Arduino
```

### Catu Daya (Power Supply)
* **Gunakan DC-DC Step Down / Buck Converter (LM2596 / MP1584)**:
  - Input: Tegangan ACC Kunci Kontak (+12V s.d. +14.4V) & Ground Aki.
  - Output: Diatur ke **5.0V stabil**, hubungkan ke pin **5V** Arduino Nano.
  - **JANGAN** menghubungkan 14.4V aki langsung ke pin `VIN` Arduino Nano!

---

## 🕹️ Panduan Penggunaan & Menu 2 Tombol

### 1. Tampilan Utama (Dashboard)
- **Sisi Kiri**: Suhu mesin saat ini (contoh: `94.5°C`).
- **Sisi Kanan**: Status `FAN ON` / `FAN OFF`, serta setpoint `ON: 96C` dan `OFF: 90C`.
- **Masuk Menu**: Tekan dan tahan **Tombol A** selama 1 detik.

### 2. Menu 1: Set Suhu Fan ON (Batas Atas)
- Tombol A (Tekan Cepat): Tambah batas atas (+1°C)
- Tombol B (Tekan Cepat): Kurang batas atas (-1°C)
- Tombol A (Tekan Tahan): Simpan dan pindah ke Menu 2
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 3. Menu 2: Set Suhu Fan OFF (Batas Bawah)
- Tombol A (Tekan Cepat): Tambah batas bawah (+1°C)
- Tombol B (Tekan Cepat): Kurang batas bawah (-1°C)
- Tombol A (Tekan Tahan): Simpan dan pindah ke Menu Kalibrasi
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 4. Menu 3: Kalibrasi Offset Suhu
- Pasang scanner OBD2 (ELM327) di soket OBD mobil untuk membaca live data suhu ECU asli.
- Bandingkan angka suhu di OLED dengan suhu di scanner.
- Tombol A (Tekan Cepat): Naikkan offset (+1°C)
- Tombol B (Tekan Cepat): Turunkan offset (-1°C)
- Tekan Tahan Tombol A/B: Simpan ke EEPROM dan kembali ke Dashboard.

*Catatan: Jika didiamkan selama 10 detik di dalam menu mana pun, setelan akan tersimpan otomatis dan kembali ke Dashboard.*

---

## 💻 Kompilasi & Upload (PlatformIO)

### Kompilasi Program:
```bash
pio run
```

### Upload ke Arduino Nano (Standar / Bootloader Baru):
```bash
pio run -t upload
```

### Upload ke Arduino Nano Clone (Old Bootloader):
Jika muncul error *programmer is not responding / stk500_recv*, gunakan environment bootloader lama:
```bash
pio run -e nanoatmega328_oldbootloader -t upload
```
