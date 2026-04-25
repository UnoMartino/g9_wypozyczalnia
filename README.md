# Grupa 9 - Wypożyczalnia samochodów

---

## Build

Projekt korzysta z customowego systemu budowania opertego na `nob.h`. Pozwala to pominąć grzebanie w skompilokwanym CMake.
Skrypt sam pobiera pliki `.cpp` z katalogu `src/` i odtwarza strukturę w `build/obj/`

### Linux / macOS
```sh
./build.sh (flags) <action> <mode>
```
Przykład: `./build.sh -j8 run debug`
### Windows 

1. Wymagany jest kompilator `gcc` i `g++` (najlepiej zainstalowany przez MSYS2/MinGW-w64) 
2. System powinien zawierać globalną zmienną CC oraz CXX, która wskazuje scieżkę kompilatora C i C++

```cmd
.\build.bat (flags) <action> <mode>
```
Przykład: `.\build.bat -j8 run debug`


### Flagi

- `-jX` - tworzy X wątków i kompiluje pliki, co znacznie przyspiesza kompilowanie projektu

### Flagi

- `-jX` - tworzy X wątków i kompiluje pliki, co znacznie przyspiesza kompilowanie projektu

### Parametry 

**action:**
- `build` - domyślnie, kompiluje i linkuje projekt, tworzac plik wykonywalny w folderze `build/`
- `run` - buduje projekt (jeśli są zmiany) i od razu uruchamia plik wykonywalny. Wszystkie parametry podane po `<mode>` zosataną bezpośrednio przekazane
do programu
- `clean` - czyści wszystko z `build/obj/` oraz sam plik wykonywalny

**mode:**
- `debug` - domyślnie, kompiluje program wraz z flagami do debugowania i wyswietlania ostrzeżeń, a także nie stosuje optymaliacji
- `release` - pomija flagi debugowania, ostrzeżenia, a także stosuje optymalilzacje na najwyższym poziomie

---

## Diagram programu
![Diagram programu](docs/img/diagram.jpg)

## Skład zespołu
- Marcin Madanowicz
- Oskar Strzelecki
- Sandra Wróblewska
- Jakub Zarębski
