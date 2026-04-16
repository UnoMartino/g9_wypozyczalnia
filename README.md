# Grupa 9 - Wypożyczalnia samochodów

---

## Build

Projekt korzysta z customowego systemu budowania opertego na `nob.h`. Pozwala to pominąc grzebanie w CMake.
Skrypt sam pobiera pliki `.cpp` z katalogu `src/` i odtwarza strukturę w `build/obj/`

### Linux / macOS
```sh
./build.sh <action> <mode>
```
### Windows 
TBD

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

---

## Diagram klas programu 
![Diagram klass](docs/img/class_diagram.png)

### Opis pszczególnych klas

**Application** - główna klasa przechowująca dane i metody potrzebne do wykonywania aplikacji. Ładuje i przechowuje listę pojazdów w bazie oraz listę użytkowników w bazie. Odczytuje oraz zapisuje dane na dysk. A także przechowuje obiekt klasy odpowiedzialny za interfejs. 

**Vehicle** - przechowuje ogólne informacje o pojeździe i jego stanie. 

**Car, Motorcycle, Truck** - klasy dziedziczące po `Vehicle`, są uzupełnieniem danych, które pasują wyłącznie do konkretnej klasyfikacji pojazdu. 

**User** - przechowuje ogólne informacje o użytkowniku t.j.: nazwa użytkownika czy hasło oraz różne flagi (np.: `is_admin`).

**View** - klasa odpowiedzialna za tworzenie oraz obsługę elementów interfejsu (TUI) 

---

## Skład zespołu
- Marcin Madanowicz
- Oskar Strzelecki
- Sandra Wróblewska
- Jakub Zarębski
