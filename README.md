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
4. **Kalibrasi 3-Tingkat Presisi Tinggi (Resolusi 0.1°C & Linear Blending)**:
   - Pengaturan batas atas (Fan ON), batas bawah (Fan OFF), dan 3 tingkat offset kalibrasi suhu (`60-80°C`, `80-90°C`, dan `>90°C`) dengan step presisi **0.1°C**.
   - Dilengkapi transisi interpolasi linier halus (*smooth blending*) di sekitar batas 80°C dan 90°C agar pembacaan suhu tidak melompat dan sangat responsif di zona kerja tinggi.
   - Layar menampilkan live suhu mesin bersamaan dengan nilai offset saat proses kalibrasi untuk memudahkan pencocokan dengan scanner OBD2.
   - Semua setelan tersimpan aman di **EEPROM** dengan verifikasi checksum (tidak hilang saat kunci kontak dimatikan).
5. **Auto-Sleep Layar OLED (Anti Burn-In & Hemat Daya)**:
   - Layar otomatis padam jika tidak ada aktivitas (waktu tunggu dapat diatur dari **1 menit hingga 30 menit**).
   - Layar menyala kembali seketika saat tombol ditekan (klik pertama hanya membangunkan layar tanpa mengubah setelan).
   - Perlindungan otomatis: Layar akan langsung menyala sendiri jika terjadi sensor error (fail-safe) atau saat kipas radiator mulai aktif menyala.
6. **Kurva Sensor OEM Honda**:
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
| **Modul Relay Fan** | `D7` | Masuk ke pin `IN` modul relay (Active LOW) |

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
- Tombol A (Tekan Tahan): Simpan dan pindah ke Menu Kalibrasi 1 (60-80°C)
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 4. Menu 3: Kalibrasi Offset 1 (Zona Sedang 60-80°C)
- Layar menampilkan: Baris atas `CAL1:60-80C T:xx.xC` (Live suhu terkalibrasi) dan Baris bawah `[ -10.0 C]` (Offset).
- Tombol A (Tekan Cepat): Tambah offset (+0.1°C)
- Tombol B (Tekan Cepat): Kurang offset (-0.1°C)
- Tombol A (Tekan Tahan): Simpan dan lanjut ke Kalibrasi 2 (80-90°C)
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 5. Menu 4: Kalibrasi Offset 2 (Zona Menjelang Suhu Kerja 80-90°C)
- Layar menampilkan: Baris atas `CAL2:80-90C T:xx.xC` dan Baris bawah `[ -10.0 C]`.
- Tombol A (Tekan Cepat): Tambah offset (+0.1°C)
- Tombol B (Tekan Cepat): Kurang offset (-0.1°C)
- Tombol A (Tekan Tahan): Simpan dan lanjut ke Kalibrasi 3 (>90°C)
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 6. Menu 5: Kalibrasi Offset 3 (Zona Kritis Fan Bekerja >90°C)
- Sangat cocok untuk mengunci presisi suhu saat fan radiator harus aktif di 96°C.
- Layar menampilkan: Baris atas `CAL3:>90C   T:xx.xC` dan Baris bawah `[ -10.0 C]`.
- Tombol A (Tekan Cepat): Tambah offset (+0.1°C)
- Tombol B (Tekan Cepat): Kurang offset (-0.1°C)
- Tombol A (Tekan Tahan): Simpan dan lanjut ke Menu 6 (Auto-Sleep OLED)
- Tombol B (Tekan Tahan): Simpan dan langsung kembali ke Dashboard

### 7. Menu 6: Auto-Sleep Layar OLED (1 - 30 Menit)
- Mengatur durasi waktu tunggu hingga layar OLED otomatis padam saat tidak digunakan (mencegah burn-in OLED).
- Layar menampilkan: Baris atas `AUTO SLEEP OLED` dan Baris bawah `[  2 MNT ]`.
- Tombol A (Tekan Cepat): Tambah waktu tunggu (+1 menit, maksimal 30 menit)
- Tombol B (Tekan Cepat): Kurang waktu tunggu (-1 menit, minimal 1 menit)
- Tekan Tahan Tombol A atau B: Simpan seluruh setelan ke EEPROM dan kembali ke Dashboard.

*Catatan:*
- *Jika didiamkan selama 10 detik di dalam menu mana pun, setelan akan tersimpan otomatis dan kembali ke Dashboard.*
- *Saat layar sedang dalam mode tidur (padam), menekan tombol apa pun akan langsung membangunkan layar tanpa mengubah setelan.*
- *Layar akan otomatis menyala sendiri jika terjadi kondisi darurat (sensor lepas/korslet) atau saat kipas radiator menyala.*

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
