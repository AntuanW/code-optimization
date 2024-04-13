# Antoni Wójcik, 12.04.2024r.
## Optymalizacja Kodu Na Różne Architektury, gr. 5
## Zadanie domowe nr 1

### I Parametry procesora
| Parametr | Wartość |
|:---:|:---:|
| Producent | Apple |
| Model | Apple M1 |
| Architektura | ARM |
| Mikroarchitektura| "Firestorm" (rdzenie wydajnościowe) | 
| | "Icestorm" (rdzenie energooszczędne) |
| Rdzenie | 8 (4 wydajnościowe + 4 energooszczędne) |
| Wątki | 8 |
| Max. częstotliwość | 3.2 GHz (rdzenie wydajnościowe) |
| | 2.06 (rdzenie energooszczędne) |
| L1 cache | 192+128 KB /rdzeń (rdzenie wydajnościowe) |
| | 128+64 KB /rdzeń (rdzenie energooszczędne) | 
| L2 cache | 12 MB (rdzenie wydajnościowe) |
| | 4 MB (rdzenie energooszczędne) |
| L3 cahce (last level cahce) | 8 MB |
| GFlops | 2 290 (FP32 Single Precision) |
| GFlops/rdzeń | 286.25 |

[Źródło: Wikipedia (niestety bardzo ciężko o porządną dokumentacje od Apple)](https://en.wikipedia.org/wiki/Apple_M1#cite_note-anandtech-1) <br>
[Źródło: Cpu-monkey](https://www.cpu-monkey.com/en/cpu-apple_m1) <br>

### II Zmiany wynikające z architektury procesora
Aby tutorial działał poprawnie, dokonałem następujących zmian w `makefile`: <br>
Zmieniłem
```MakeFile
...
CC         := gcc
LINKER     := $(CC)
CFLAGS     := -O2 -Wall -msse3
LDFLAGS    := -lm
...
```
Na:
```MakeFile
...
CC         := clang
LINKER     := $(CC)
CFLAGS     := -O2 -Wall -target arm64-apple-macos
LDFLAGS    := -lm
...
```

### III Optymalizacje

### IV Wyniki

### V Podsumowanie