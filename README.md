MiniOS
Minimalistyczny system terminalowy dla Raspberry Pi Pico W

Informacja	Wartość
Wersja dokumentacji	0.3
Wersja docelowa firmware'u	MiniOS v0.1
Platforma sprzętowa	Raspberry Pi Pico W
Mikrokontroler	RP2040
Języki programowania	C i C++
SDK	Raspberry Pi Pico SDK
System budowania	CMake
Interfejs użytkownika	Terminal tekstowy przez USB
Stan projektu	Rozwój firmware'u, stabilizacja i refaktoryzacja
1. Opis projektu
MiniOS to autorski, minimalistyczny system firmware przeznaczony dla Raspberry Pi Pico W. Jego celem jest przekształcenie mikrokontrolera w niewielki komputer sterowany za pomocą poleceń tekstowych.

Użytkownik komunikuje się z urządzeniem przez terminal USB podłączony do komputera. Pico W wykonuje kod, obsługuje sprzęt, mierzy czas działania, przetwarza polecenia i docelowo umożliwia korzystanie z systemu plików, sieci oraz prostych aplikacji.

Projekt rozwijany jest etapami. Priorytetem jest stabilność podstawowego systemu, czytelna architektura oraz możliwość późniejszego rozszerzania funkcjonalności bez konieczności przebudowy całego firmware'u.

MiniOS nie korzysta obecnie z FreeRTOS. Podstawą jest własna architektura firmware'u z pętlą główną, modułami usług, systemem zdarzeń i obsługą timerów.

2. Założenia projektu
Główne założenia MiniOS:

sterowanie urządzeniem przez terminal USB;
brak konieczności stosowania zewnętrznego wyświetlacza;
modularna architektura oparta na C i C++;
rozdzielenie powłoki, kernela, usług, diagnostyki i obsługi sprzętu;
komunikacja między modułami za pomocą interfejsów i zdarzeń;
możliwość późniejszego dodania systemu plików i skryptów;
przygotowanie architektury pod zewnętrzną pamięć Flash oraz karty microSD;
możliwość uruchomienia podstawowego systemu bez opcjonalnych modułów sprzętowych;
stopniowe wprowadzanie kolejnych funkcji i testowanie ich na fizycznym urządzeniu.
3. Platforma sprzętowa
Docelową platformą jest Raspberry Pi Pico W.

Zasób	Charakterystyka
Mikrokontroler	RP2040
Rdzenie CPU	2 × ARM Cortex-M0+
Taktowanie	Docelowo 133 MHz
SRAM	264 KB
Flash	Nominalnie 2 MB na płytce
Wi-Fi	CYW43439
Interfejs użytkownika	USB i terminal tekstowy
GPIO	Wejścia i wyjścia cyfrowe
Interfejsy sprzętowe	SPI, I2C, UART, PWM, ADC
Podane parametry opisują platformę, a nie wyniki diagnostyki wykonywanej przez MiniOS. Rzeczywiste konfiguracje, wykorzystanie pamięci i dostępność funkcji należy weryfikować w kodzie i na urządzeniu.

4. Punkt kontrolny projektu
MiniOS jest rozwijany na podstawie określonego punktu kontrolnego, czyli checkpointu projektu.

Checkpoint oznacza konkretny stan kodu źródłowego, architektury i dokumentacji, od którego kontynuowane są dalsze prace.

Zasady pracy z checkpointem
Przed rozpoczęciem większej zmiany należy ustalić aktualny stan kodu.
Nie należy zakładać, że planowana funkcja jest już zaimplementowana.
Nie należy bez uzasadnienia zmieniać istniejących interfejsów, działających komend ani przydziału GPIO.
Zmiany architektury należy opisać przed ich wdrożeniem.
Po zmianach należy wykonać kompilację i testy odpowiednie dla danego etapu.
Wyniki testów na urządzeniu należy odróżniać od samego faktu kompilacji.
Dokumentacja musi odzwierciedlać rzeczywisty stan implementacji.
Zaleca się przechowywanie checkpointów w repozytorium Git poprzez commity i oznaczone wersje. W ten sposób można odtworzyć wcześniejszy stan projektu i porównać kolejne etapy rozwoju.

Ważne: sam dokument README nie identyfikuje konkretnego commita ani kopii kodu. Dokładny checkpoint należy ustalić na podstawie repozytorium, identyfikatora commita lub przekazanego archiwum projektu.

5. Aktualna architektura
Kod został podzielony na moduły odpowiedzialne za określone zadania.

Przykładowa struktura aktualnego projektu:

MiniOS/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── MiniOS.md
└── src/
    ├── main.c
    ├── kernel/
    │   ├── event.c
    │   ├── event.h
    │   ├── timer.c
    │   └── timer.h
    ├── monitor/
    │   ├── memory.c
    │   └── memory.h
    ├── services/
    │   ├── morse.c
    │   ├── morse.h
    │   ├── uptime.c
    │   └── uptime.h
    ├── shell/
    │   ├── history_c.cpp
    │   ├── history_c.h
    │   ├── history.cpp
    │   ├── history.h
    │   ├── shell_c.h
    │   ├── shell.cpp
    │   └── shell.hpp
    ├── shell_command_event.h
    ├── shell_command_handlers.c
    └── system/
        ├── core.c
        └── core.h
Struktura przedstawia dotychczasowy podział kodu. Nowe katalogi, takie jak filesystem/, storage/ i script/, zostaną dodane w miarę implementowania kolejnych etapów.

5.1. main.c
Główny punkt wejścia firmware'u.

Odpowiada za inicjalizację systemu, przygotowanie powłoki, odbieranie znaków z terminala, obsługę historii poleceń oraz cykliczne wywoływanie timer_poll().

Program działa obecnie w pętli głównej. Nie ma jeszcze potwierdzonego pełnego schedulera zadań.

5.2. kernel/event.c
Podstawowy system zdarzeń umożliwiający rejestrację handlerów i synchroniczne wywoływanie ich przez event_emit().

Przykładowe zdarzenia:

EVENT_BOOT
EVENT_SYSTEM_READY
EVENT_USB_CONNECTED
EVENT_USB_DISCONNECTED
EVENT_TERMINAL_CONNECTED
EVENT_TERMINAL_DISCONNECTED
EVENT_COMMAND
EVENT_REBOOT
EVENT_TIMER_EXPIRED
Mechanizm jest synchroniczny. Nie stanowi kolejki zdarzeń ani systemu wielowątkowego.

5.3. kernel/timer.c
Moduł obsługi timerów. Pętla główna wywołuje timer_poll(), aby sprawdzać upływ czasu bez konieczności stałego blokowania programu.

Obsługa wielu timerów, ich wygaszanie i poprawność emisji zdarzeń wymagają weryfikacji na podstawie aktualnej implementacji oraz testów.

5.4. shell/
Powłoka MiniOS jest zaimplementowana w C++.

Odpowiada za rozpoznawanie poleceń, argumentów oraz kierowanie ich do odpowiednich handlerów. Historia poleceń jest oddzielnym modułem.

Interfejsy z końcówką _c umożliwiają współpracę kodu C z kodem C++ za pomocą interfejsów zgodnych z C.

5.5. shell_command_handlers.c
Moduł łączący polecenia powłoki z operacjami systemowymi.

Wykorzystuje zdarzenia do przekazywania żądań wykonania komend. Dzięki temu parser nie musi bezpośrednio implementować każdej operacji sprzętowej i diagnostycznej.

5.6. services/
Zawiera usługi systemowe, w tym obsługę czasu działania oraz nadawanie wiadomości alfabetem Morse'a.

5.7. monitor/memory.c
Moduł podstawowego monitorowania pamięci.

Obecnie prezentowane informacje obejmują między innymi całkowitą SRAM, przybliżony zakres wykorzystania pamięci statycznej i parametry obszaru sterty.

Nie jest to jeszcze pełny Memory Manager. Rzeczywiste wykorzystanie sterty, stosu i wszystkich obszarów pamięci wymaga osobnych mechanizmów pomiarowych.

5.8. system/core.c
Moduł odpowiedzialny za podstawową inicjalizację systemu, sekwencję startową i prezentację informacji systemowych.

Dokładny zakres testów startowych należy potwierdzić w implementacji boot_sequence().

6. Powłoka i dostępne komendy
Terminal tekstowy przez USB jest podstawowym interfejsem użytkownika.

Dotychczasowa tabela komend obejmuje:

Komenda	Przeznaczenie
help	Pomoc
info	Informacje o systemie
version	Wersja MiniOS
uptime	Czas działania
timer	Test timera
timer2	Drugi test timera
clear	Czyszczenie terminala
led on	Włączenie LED
led off	Wyłączenie LED
echo	Wyświetlanie argumentów
history	Historia poleceń
morse	Nadawanie tekstu Morse'em
mem	Informacje o pamięci
diag	Diagnostyka systemu
reboot	Restart urządzenia
Tabela opisuje komendy uwzględnione w dotychczasowej dokumentacji kodu. Nie oznacza, że wszystkie zostały przetestowane na fizycznym Pico W.

Obsługa terminala
Dotychczasowy interfejs przewiduje:

odbieranie znaków przez standardowe wejście;
zatwierdzanie poleceń klawiszem Enter;
obsługę Backspace;
bufor wejściowy o rozmiarze 64 bajtów;
historię do 10 poleceń;
nawigację historią za pomocą strzałek góra/dół.
Do sprawdzenia pozostaje między innymi zachowanie po rozłączeniu USB, przepełnienie bufora i edycja polecenia pobranego z historii.

7. Docelowa architektura systemu plików
System plików jest planowanym rozszerzeniem. Od początku projektujemy go tak, aby nie był uzależniony od jednego rodzaju pamięci.

Docelowo MiniOS będzie wykorzystywał VFS (Virtual File System), czyli warstwę pośrednią pomiędzy powłoką a fizycznymi nośnikami.

Shell MiniOS
    |
    v
VFS
    |
    +-- Internal Flash
    |
    +-- External Flash
    |
    +-- microSD
VFS będzie odpowiadał za wspólne operacje plikowe, obsługę ścieżek, katalogów, błędów oraz wybór właściwego backendu.

7.1. Planowane nośniki
Nośnik	Przeznaczenie	Status
Wewnętrzna Flash Pico W	Firmware i wybrane dane systemowe	Wymaga zaprojektowania bezpiecznego obszaru danych
Zewnętrzna Flash	Dodatkowe dane i pliki systemowe	Planowana
Karta microSD	Pliki użytkownika, skrypty i logi	Planowana
Brak zewnętrznej Flash lub karty SD nie może uniemożliwiać uruchomienia podstawowego systemu.

Wewnętrzna Flash zawiera firmware, dlatego nie wolno zapisywać danych użytkownika w obszarze zajmowanym przez kod ani w obszarach zarezerwowanych przez konfigurację linkera.

7.2. Docelowe katalogi
/
├── system/
│   ├── config/
│   └── scripts/
├── bin/
├── apps/
├── home/
│   └── user/
├── data/
├── logs/
└── tmp/
Jest to struktura docelowa. Katalogi nie muszą od razu istnieć na każdym nośniku.

7.3. Planowane polecenia plikowe
ls — wyświetlanie zawartości katalogu;
cd — zmiana bieżącego katalogu;
cat — odczyt pliku;
write — zapis danych;
rm — usuwanie pliku.
Komendy zostaną dodane po przygotowaniu interfejsu VFS i pierwszego backendu.

8. Zewnętrzna Flash i karta microSD
Projekt przewiduje możliwość podłączenia dodatkowych nośników przez GPIO.

Wstępna koncepcja zakłada:

SPI0 jako możliwą magistralę zewnętrznej pamięci Flash;
SPI1 jako możliwą magistralę czytnika microSD;
osobne linie CS dla urządzeń SPI;
niezależne wykrywanie dostępności nośników.
Przydział magistral i pinów nie jest jeszcze ostateczny.

Przed implementacją należy ustalić dokładne modele modułów, wymagania napięciowe, sterowniki, obsługiwane tryby komunikacji i ewentualne konflikty z innymi urządzeniami.

Nie ustalono jeszcze rozmiarów partycji ani ostatecznego podziału pamięci wewnętrznej.

9. MiniScript
MiniScript to planowany, prosty interpreter skryptów dla MiniOS.

Nie będzie pełnym odpowiednikiem Basha. Jego celem jest umożliwienie automatycznego wykonywania wybranych poleceń systemowych.

Przykładowy przyszły skrypt hello.msh:

echo "MiniOS script started"
info
uptime
echo "Script completed"
Przykładowe użycie:

mini> ls /home/user
mini> cat /home/user/hello.msh
mini> run /home/user/hello.msh
Pierwsza wersja będzie obsługiwała wybrane komendy MiniOS. Interpreter ma odczytywać skrypt linia po linii, aby ograniczyć zużycie SRAM.

Zmienne, warunki, ograniczone pętle i argumenty skryptów mogą zostać dodane później.

Uruchamianie niezależnych programów binarnych z pamięci Flash nie jest częścią pierwszej wersji MiniScript.

10. Sprzęt diagnostyczny
Planowany panel diagnostyczny:

GPIO	Element	Funkcja
GP2	Czerwona LED	ERROR
GP3	Żółta LED	STORAGE
GP4	Niebieska LED	WIFI
GP5	Zielona LED	SYSTEM
GP6	BTN1	Reset/anulowanie lub funkcja systemowa
GP7	BTN2	Tryb systemowy
GP8	BTN3	Diagnostyka
GP9	BTN4	Wi-Fi
Przydział jest planem, a nie potwierdzeniem fizycznego montażu.

Każda zewnętrzna LED powinna mieć rezystor ograniczający prąd. Przyciski mogą korzystać z wewnętrznych rezystorów pull-up.

Pico W używa logiki 3,3 V. Nie wolno podawać 5 V bezpośrednio na GPIO.

Obsługa diody pokładowej musi być oddzielona od sterowania zewnętrznymi LED-ami. W Pico W dioda pokładowa jest obsługiwana przez układ CYW43439, dlatego nie należy zakładać zgodności ze sterowaniem GPIO25 stosowanym w klasycznym Pico.

11. Pamięć i wydajność
Dotychczasowy punkt odniesienia dla rozmiaru programu:

Sekcja	Rozmiar
text	46 292 B
data	0 B
bss	4 720 B
Wartości odpowiadają około 45,21 KiB dla text oraz 4,61 KiB dla bss.

Nie opisują one pełnego wykorzystania SRAM w czasie działania ani przyszłych wymagań VFS, sterowników i MiniScript.

Przy kolejnych etapach należy porównywać wyniki kompilacji, sprawdzać mapę linkera i mierzyć zużycie pamięci na urządzeniu.

12. Kernel i scheduler
Aktualna architektura korzysta z pętli głównej, systemu zdarzeń i timerów.

Docelowo planowany jest prosty scheduler kooperacyjny:

Kernel
├── Shell task
├── System task
├── LED task
├── WiFi task
├── Storage task
└── Application task
Pierwsza wersja nie musi realizować przełączania kontekstu procesora. Może polegać na regularnym wywoływaniu funkcji zadań.

Długotrwałe operacje powinny być projektowane tak, aby nie blokowały niepotrzebnie obsługi terminala.

Pełny scheduler nie jest obecnie potwierdzoną funkcją systemu.

13. Wi-Fi
Raspberry Pi Pico W zawiera układ CYW43439, ale samo jego istnienie nie oznacza gotowości połączenia sieciowego.

Planowane komendy:

wifi status
wifi scan
wifi connect
wifi disconnect
ip
ping
Implementacja będzie wymagała odpowiednich komponentów Raspberry Pi Pico SDK i stosu sieciowego.

Status połączenia powinien wynikać z rzeczywistego stanu sterownika, a nie z wartości wpisanej na stałe.

14. Aplikacje
W dalszej przyszłości MiniOS może obsługiwać proste aplikacje:

mini> apps

calculator
editor
monitor
network
Docelowo może pojawić się także polecenie run dla aplikacji, jednak należy rozróżnić wykonywanie skryptów MiniScript od uruchamiania niezależnych programów binarnych.

Model aplikacji, format programów i sposób ich ładowania pozostają do zaprojektowania.

15. Kompilacja i uruchomienie
Projekt wykorzystuje Raspberry Pi Pico SDK oraz CMake.

Przykładowy schemat konfiguracji i budowania:

cmake -S . -B build
cmake --build build
Powyższe polecenia są ogólnym przykładem. Dokładna procedura zależy od konfiguracji projektu, sposobu udostępnienia Pico SDK i wymagań środowiska kompilacji.

Po udanej kompilacji należy:

Wgrać firmware na Raspberry Pi Pico W.
Podłączyć urządzenie przez USB.
Otworzyć terminal szeregowy zgodny z konfiguracją firmware'u.
Sprawdzić komunikaty startowe.
Przetestować podstawowe komendy.
Sprawdzić restart, historię i zachowanie po rozłączeniu terminala.
Kompilacja nie zastępuje testów na fizycznym urządzeniu.

16. Znane ograniczenia
Do weryfikacji pozostają między innymi:

resetowanie flagi has_command w pętli terminala;
zachowanie po rozłączeniu i ponownym podłączeniu USB;
obsługa granic bufora wejściowego i historii;
poprawność obsługi wielu timerów;
zakres testów wykonywanych podczas boot_sequence();
zgodność informacji systemowych z rzeczywistą konfiguracją;
wiarygodność raportu pamięci;
limity rejestracji handlerów zdarzeń;
wpływ długotrwałych operacji na pętlę główną;
rozdzielenie LED-a pokładowego i zewnętrznych LED-ów;
kompilacja całego projektu po refaktoryzacji;
testy wszystkich aktualnych komend na urządzeniu.
Nie należy traktować wymienionych ograniczeń jako potwierdzenia, że zostały już usunięte.

17. Plan rozwoju
Etap A — stabilizacja
kompilacja całego projektu;
poprawienie znanych błędów;
test terminala i historii;
test komend i timerów;
sprawdzenie restartu.
Kryterium ukończenia: aktualne funkcje działają powtarzalnie.

Etap B — MiniOS v0.1
uporządkowany ekran startowy;
wiarygodne informacje systemowe;
podstawowe komendy;
poprawna obsługa USB;
sterowanie LED-ami;
czytelne błędy.
Etap C — panel sprzętowy
montaż LED-ów i przycisków;
test GPIO;
diagnostyka sprzętu;
obsługa trybu diagnostycznego.
Etap D — VFS i pamięć masowa
wspólny interfejs systemu plików;
wybór modelu backendu;
bezpieczny obszar danych wewnętrznej Flash;
podstawowe operacje plikowe;
obsługa braku nośnika i błędów I/O.
Etap E — MiniScript
odczyt skryptów linia po linii;
wykonywanie wybranych poleceń;
kontrola błędów i ograniczeń pamięciowych.
Etap F — kernel i scheduler
dopracowanie timerów;
zadania kooperacyjne;
ograniczenie blokowania pętli głównej;
diagnostyka błędów.
Etap G — rozszerzenia sprzętowe i Wi-Fi
sterownik zewnętrznej Flash;
sterownik microSD;
obsługa CYW43439;
status sieci i podstawowe operacje.
Etap H — aplikacje i dalsze rozszerzenia
model aplikacji;
przykładowe programy;
analiza potrzeby dodatkowej pamięci;
ewentualne ładowanie niezależnych programów.
Etapy mogą być porządkowane ponownie na podstawie wyników testów i rzeczywistych wymagań sprzętowych.

18. Zasady współpracy przy rozwoju
Każda większa zmiana powinna zawierać:

opis celu;
wskazanie zmienianych modułów;
implementację;
kompilację;
test na urządzeniu;
aktualizację dokumentacji;
zapis checkpointu w repozytorium.
Nie należy jednocześnie wprowadzać kilku dużych zmian architektonicznych bez możliwości ich niezależnego przetestowania.

Priorytetem pozostaje stabilny firmware. Kolejne funkcje mają być dodawane w sposób kontrolowany, z zachowaniem zgodności istniejących interfejsów.

19. Dokumentacja projektu
Zalecany podział dokumentacji:

README.md — ogólny opis projektu, architektura, kompilacja i plan rozwoju;
docs/MiniOS.md — szczegółowy stan implementacji, ograniczenia techniczne i lista kontrolna;
docs/architecture.md — docelowa architektura modułów, VFS i backendów;
docs/hardware.md — schematy połączeń, GPIO i wykaz podzespołów;
docs/testing.md — procedury testowe i wyniki testów.
Dokumenty powinny być aktualizowane wraz z kodem, tak aby repozytorium stanowiło wiarygodny punkt odniesienia dla kolejnych checkpointów.

20. Podsumowanie
MiniOS to rozwijany etapami, minimalistyczny system firmware dla Raspberry Pi Pico W, którego podstawowym interfejsem jest terminal USB.

Obecna architektura obejmuje powłokę, system zdarzeń, obsługę timerów, usługi systemowe, podstawowy monitor pamięci i moduły inicjalizacyjne.

VFS, backendy pamięci, MiniScript, scheduler, Wi-Fi i uruchamianie aplikacji są elementami planu rozwoju i nie powinny być przedstawiane jako ukończone bez potwierdzenia implementacji i testów.

Najważniejsza zasada projektu: najpierw stabilizujemy aktualny checkpoint, potem rozszerzamy system.

Raspberry Pi Pico W jest komputerem MiniOS, a terminal USB pełni rolę jego podstawowego ekranu i interfejsu użytkownika.

Co dalej?
Zapisz dokument jako README.md w głównym katalogu repozytorium.

Zachowaj szczegółową dokumentację techniczną w docs/MiniOS.md.

Po ustaleniu aktualnego stanu kodu dodaj identyfikator checkpointu, np. hash commita Git, żeby było jasne, od którego stanu kontynuujemy prace.