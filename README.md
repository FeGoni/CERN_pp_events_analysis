# Symulacja i analiza rozkładów cząstek w zderzeniach pp przy √s = 13.6 TeV

Autor: Paweł Zakrzewski



Projekt obejmuje generację 100 000 zderzeń proton–proton w trybie minimum bias (Pythia 8), analizę cząstek stanu końcowego w C++/ROOT oraz przygotowanie wykresów. Wyznaczane są rozkłady krotności na zdarzenie (N), pędu poprzecznego (pT), pseudorapidity (η) i kąta azymutalnego (φ) dla wszystkich cząstek naładowanych oraz π±, K±, p oraz p̄

## Zawartość repozytorium

|Plik|Opis|
|-|-|
|`generate.cc`, `generate.cmnd`|program generujący zdarzenia i jego konfiguracja|
|`analysis.cc`|program tworzący histogramy|
|`draw_plots.C`|makro ROOT rysujące wykresy|
|`analysis.root`|wynikowe histogramy dla 100 000 zdarzeń|
|`plots/`|wykresy w formatach PNG i PDF|
|`pres.pdf`|prezentacja zawierająca wykresy|
|`Makefile`|kompilacja i uruchamianie kolejnych etapów|

Plik `events.root` (ok. 225 MB) nie znajduje się w repozytorium. Można go odtworzyć poleceniem `make events`

## Wymagania

* ROOT 6 (testowano na wersji 6.40.04)
* Pythia 8 (testowano na wersji 8.312)
* kompilator C++ zgodny z instalacją ROOT-a (testowano na `g++` 15.3)
* GNU Make

Kod jest zgodny ze standardem C++17 (sprawdzono flagami `-std=c++17 -pedantic`). Standard kompilacji jest przejmowany z `root-config`, aby był zgodny z użytą instalacją ROOT-a. W testowanym środowisku jest to `-std=c++20`

Projekt testowano na Linuksie x86-64 (WSL2).

### Wariant 1: zainstalowane ROOT i Pythia

Polecenia `root`, `root-config` i `pythia8-config` muszą być dostępne w `PATH`. Przed uruchomieniem należy wskazać bazę danych Pythii:

```bash
export PYTHIA8DATA=$(pythia8-config --xmldoc)
```

### Wariant 2: środowisko conda

```bash
conda create -n hep_env -c conda-forge root=6.40.04 pythia8=8.312 compilers make
conda activate hep_env
```

Polecenie `conda activate` ustawia `PYTHIA8DATA` automatycznie.

## Kompilacja

```bash
make
```

Polecenie tworzy programy `generate` i `analysis`.

## Uruchomienie

Cały łańcuch, wraz z kompilacją, uruchamia polecenie:

```bash
make task
```

Poszczególne etapy można też uruchomić osobno:

|Polecenie|Etap|Wejście|Wyjście|
|-|-|-|-|
|`make events`|generacja zdarzeń|`generate.cmnd`|`events.root`|
|`make hist`|analiza|`events.root`|`analysis.root`|
|`make plots`|wykresy|`analysis.root`|`plots/`|

Polecenie `make task` za każdym razem generuje zdarzenia od nowa. Do odtworzenia samych wykresów z dołączonego pliku `analysis.root` wystarczy `make plots`. Polecenie `make clean` usuwa skompilowane programy

Programy można również uruchomić bezpośrednio:

```bash
./generate generate.cmnd events.root
./analysis events.root analysis.root
root -l -b -q 'draw_plots.C("analysis.root", "plots")'
```

## Generacja zdarzeń

Konfiguracja znajduje się w pliku `generate.cmnd`

|Ustawienie|Wartość|Opis|
|-|-|-|
|`Beams:idA`, `Beams:idB`|2212, 2212|zderzenia proton–proton|
|`Beams:eCM`|13600|√s = 13.6 TeV|
|`SoftQCD:inelastic`|on|minimum bias|
|`Random:setSeed`, `Random:seed`|on, 99999||
|`Main:numberOfEvents`|100000|liczba zapisywanych zdarzeń|
|`Main:timesAllowErrors`|100|dopuszczalna liczba nieudanych wywołań `pythia.next()`|

Zapisywane są wyłącznie cząstki stanu końcowego ('particle.isFinal()')

### Format pliku `events.root`

Drzewo `events` zawiera jeden wpis na zdarzenie. Każda gałąź jest wektorem z jednym elementem na cząstkę

|Gałąź|Typ|Opis|
|-|-|-|
|`pdg`|`vector<int>`|kod PDG cząstki|
|`charge`|`vector<int>`|ładunek|
|`pt`|`vector<float>`|pęd poprzeczny \[GeV]|
|`eta`|`vector<float>`|pseudorapidity|
|`phi`|`vector<float>`|kąt azymutalny \[rad]|



## Analiza

Program `analysis` odczytuje drzewo `events` i dla każdej z ośmiu kategorii wypełnia cztery histogramy `TH1D`, co daje łącznie 32 histogramy





## Wykresy

Makro `draw_plots.C` odczytuje `analysis.root`, rysuje każdy wykres na osobnym canvasie i zapisuje go w formatach PNG i PDF. Łącznie powstają 64 wykresy

|Pliki w `plots/`|Zawartość|Liczba wykresów|
|-|-|-|
|`hN_X`, `hPt_X`, `hEta_X`, `hPhi_X`|histogramy z ramką statystyk|32|
|`hN_X_lin`, `hPt_X_lin`|rozkłady N i pT w skali liniowej|16|
|`<hN\|hPt\|hEta\|hPhi>_<all_charged\|pions\|kaons\|protons>`|porównania z legendą: all i ch, π⁺ i π⁻, K⁺ i K⁻, p i p̄|16|
|`all_plots.pdf`|wszystkie wykresy w jednym pliku|64 strony|

Rozkłady N i pT są rysowane w skali logarytmicznej oraz liniowej dla lepszej analizy

Wykres `hN_all_charged` jest rysowany punktami dla czytelności, a pozostałe jako histogramy

Oś X wykresów `hN` jest przycinana do zakresu zawierającego dane, np. 0–40 dla kaonów. Histogram zapisany w pliku zachowuje pełny zakres 0–400

## Odtwarzalność

Ta sama wersja Pythii i ta sama konfiguracja, w tym seed generatora, dają identyczne zdarzenia. Sprawdzono to, powtarzając pełną produkcję 100 000 zdarzeń

