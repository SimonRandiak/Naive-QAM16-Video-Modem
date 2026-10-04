# QAM16 modem – video 320×240 @ 30 fps

Prenos H.264 videa cez QAM16. Prvá fáza beží na PC (GNU Radio), neskôr STM32 generuje I/Q vzorky cez DAC do IQ modulátora (Analog Devices).

## Parametre

| Parameter | Hodnota |
|---|---|
| **Video** | 320×240, 30 fps, H.264 v MPEG-TS |
| Dátový tok videa | 500 kbit/s (GOP 60) |
| **Modulácia** | QAM16, Grayovo mapovanie (4 bity/symbol) |
| **Symbolová rýchlosť** | **300 kBd** |
| Surový bitový tok | 1,2 Mbit/s |
| FEC | RS(204,188) + konvolučný kód 3/4 |
| Réžia rámcovania (preambula, hlavička, piloty, CRC) | ~10 % |
| Užitočná kapacita pre video | **~750 kbit/s** |
| Rezerva nad videom | ~50 % |
| **RRC roll-off α** | **0,4** |
| **Šírka pásma** | **420 kHz** (±210 kHz v základnom pásme) |
| **Fs (DAC)** | **2,4 MS/s** |
| Vzoriek na symbol | 8 |
| RRC filter | rozpätie 11 symbolov → 89 koeficientov (treba overit pocet ) |
| Low pass filter | priepustné pásmo 230KHz nepriepustné pásmo 350-400 KHz |

## Príprava videa

```bash
ffmpeg -i vstup -s 320x240 -r 30 -c:v libx264 -preset ultrafast \
  -tune zerolatency -b:v 500k -maxrate 500k -bufsize 250k -g 60 \
  -f mpegts out.ts
```

Pre rýchlejšie zotavenie po chybe: `-g 30` alebo `-intra-refresh 1` (~10–15 % toku navyše).

## Prijem videa
```bash
ffplay -fflags nobuffer -flags low_delay udp://127.0.0.1:1234
```

## Štruktúra rámca
CRC plati len pre PAYLOAD
```
| SYNC 0x1ACFFC1D | len(2) | PAYLOAD (≤1316 B) | CRC32 |
```

- **Preambula:** 32–64 známych symbolov (rohové body konštelácie), slúži na AGC, časovú synchronizáciu a odhad fázy.
- **Hlavička:** číslo sekvencie, dĺžka, CRC16.
- **Payload:** napr. 7 TS paketov (7 × 188 B = 1316 B), s interleaverom.
- **Pilotné symboly:** 1 každých ~100 symbolov na sledovanie fázy.

## GNU Radio

Constellation Modulator:

- `constellation`: 16QAM, Gray mapping
- `samples_per_symbol = 8`
- `excess_bw = 0.4`

### GNU Radio – Constellation Rect. Object (16-QAM, Gray)

| Parameter               | Hodnota | Typ |
|-------------------------|---------|-----|
| ID                      | `qam16` | – |
| Symbol Map              | `[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]` | int_vector |
| Constellation Points    | `[-3-3j, -3-1j, -3+3j, -3+1j, -1-3j, -1-1j, -1+3j, -1+1j, 3-3j, 3-1j, 3+3j, 3+1j, 1-3j, 1-1j, 1+3j, 1+1j]` | complex_vector |
| Rotational Symmetry     | `4` | int |
| Real Sectors            | `4` | int |
| Imaginary Sectors       | `4` | int |
| Width Real Sectors      | `2` | int |
| Width Imaginary Sectors | `2` | int |
| Soft bits precision     | `8` | int |

Priemerny vykon Es je 10
Eb = 10/log2(4)

### Problemy
Vykon pre PLL hra velku rolu preto je signal dobre normalizovat na jednotkovy vykon aby sedelo kD gain

Pri gardner TED je rozdiel medzi akumulatorovou a indexovou verziou

Pozor pri cross product znamienku slucky a pri faze a integraly
Nakolko |a x b| da kladne cislo ale |b x a| moze dat zaporne cislo 