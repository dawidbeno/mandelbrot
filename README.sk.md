# Vykreslenie Mandelbrotovho fraktálu pomocou SSE a AVX 
Program vykresľuje Mandelbrotov fraktál s použitím špeciálnych inštrukcii procesora SSE a AVX pre značné zvýšenie rýchlosti vykresľovania. Používateľ si môže vybrať spôsob vykreslenia obrázku - použitie špeciálnych inštrukcii a tiež aj veľkosť obrázku, ktorý sa vykreslí. Okrem toho je možné spustiť testovanie, ktoré zmeria rýchlosť pri vykreslovaní rôznych veľkostí obrázkov za použitia špeciálnych inštrukcií a bez nich.

![Alt text](images/mandelbrot.png?raw=true "Mandelbrotov fraktál")

## Inštalácia
Program nevyžaduje žiadnu špeciálnu inštaláciu, iba stiahnutie kódov a ich skompilovanie príkazom:
```
make
```
alebo príkazom:
```
gcc -Wall -Wextra -O3 -msse2 -mavx mandel.c mandel_sse2.c mandel_avx.c -o mandelEXE
```

## Spustenie
Program s spúšťa pomoocu Bash skriptu:
```
./mandel [-h] [-B] [-S] [-A] [-t] [-w] [-xy num num] [-c]
```
### Prepínače
- **-h** 
	* Program vypíše help.
- **-B**
	* Vykreslenie obrázku bez použitia inštrukcií (Basic).
- **-S**
	* Vykreslenie obrázku s použitím SSE inštrukcií.
- **-A**
	* Vykreslenie obrázku s použitám AVX inštrukcií.
- **-t**
	* Vykoná testovanie, ktoré vypíše priemerný čas vykreslenia obrázkov bez použitia špeciálnych inštrukcii, s SSE a s AVX. Priemerný čas sa vypočíta z časov 10 vykreslení obrázka pre každý typ programu. Testovaie sa vykonáva pre viacero veľkostí obrázkov (N x N) so stranou 500, 1000, 2000, 3000 a 4000 pixelov."
- **-w**
	* Uloží obrázok do priečinku s programom. Názov obrázku je fractal_[Basic / SSE / AVX].png. Ak je prepínac použitý v kombinacii s -T, tak uloží len posledný (a teda najväčší) obrázok z testovania."
- **-xy num num**
	* Šírka a výška obrázku, ktorý sa vykreslí. Za prepínačom **-xy** musia nasledovať dve čísla oddelené medzerou.
- -c
	* Vyčistí priečinok - vymazanie vykreslených obrázkov a všetkých skompilovaných súborov.

## Xcode project
V priečinku **xcode_mandel_aps** je umiestnený projekt pre prostredie Xcode na macos.

## Záver
Tento projekt bol vytvorený ako školský projekt na FIIT STU v rámci predmetu Architektúra počítačových systémov.

