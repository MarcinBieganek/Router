# Router

Program implementujący prosty routing. Wykorzystuje on algorytm wektora odległości i co określony czas tworzy tablice przekazywania dla komputera na którym jest uruchomiony.
Zadanie zrealizowane w ramach przedmiotu Sieci Komputerowe na Uniwersytecie Wrocławskim w maju 2021. Autor: Marcin Bieganek.

### Działanie

Program na wejściu standardowym dostaje konfigurację sieciową komputera na którym działa w formacie:

`<adres interfejsu w formacie CIDR> distance <odległość>`
  
Np:
2
10.1.0.1/8 distance 10
192.168.5.40/24 distance 3
 
Następnie program co 15 sekund rozsyła informację o swoim obecnym wektorze odległości na adresy rozgłoszeniowe bezpośrednio podłączonych sieci. Każdy wpis z wektora to adres sieci i obecna odległość do niej. Każdy wpis jest rozsyłany w osobnym pakiecie UDP w formacie:
  
Bajty od 1 do do 5 opisują adres CIDR sieci. Bajty od 1 do 4 to adres IP zapisany w sieciowej kolejności bajtów, a 5 bajt to długość maski.
Bajty od 6 do 9 opisują odległość do tej sieci zapisaną w sieciowej kolejności bajtów.

Również co 15 sekund program wypisuję na standardowe wyjście obecną tablicę routingu. Przykład takiej tablicy:

10.1.1.0/8 distance 4 connected directly
192.168.2.0/24 distance 3 connected directly
192.168.5.0/24 distance 6 via 192.168.5.5
172.32.0.0/16 distance 10 via 10.1.1.2

Pomiędzy tymi akcjami, przez 15 sekund, program odczytuje komunikaty od innych instancji tego programu uruchomionych na komputerach połączonych sieciami z naszym komputerem i aktualizuje wektor odległości. Program nasłuchuję i wysyła komunikaty na port 54321.

### Obsługa

Projekt zawiera Makfile. Wykonując polecenie `make` możemy skompilować program.
Polecenie `make clean` czyści katalog z plików pośrednich. 
Polecenie `make distclean` czyści katalog z plików pośrednich oraz pliku wykonywalnego.

Po kompilacji program można uruchomić poleceniem `./router` i przekazać mu na standardowe wejście konfigurację sieciową komputera wraz z odległościami do sieci.

