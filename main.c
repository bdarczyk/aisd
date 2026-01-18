#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- DEFINICJE I STRUKTURY ---

// Struktura reprezentujaca wezel w drzewie Huffmana
typedef struct element_drzewa {
    char litera;
    int czestosc;
    struct element_drzewa *syn0; // lewe dziecko (bit 0)
    struct element_drzewa *syn1; // prawe dziecko (bit 1)
} element_t;

// kolejka priorytetowa zaimplementowana jako kopiec
typedef struct {
    int rozmiar_aktualny;
    element_t **tablica_wskaznikow; // dynamiczna tablica wskaznikow
} priorytetowa_kolejka_t;


// Funkcja tworzaca nowy element drzewa
element_t* utworz_element(char znak, int waga) {
    element_t* nowy = (element_t*)malloc(sizeof(element_t));
    if (!nowy) return NULL;

    // wartosci poczatkowe
    nowy->litera = znak;
    nowy->czestosc = waga;
    nowy->syn0 = NULL;
    nowy->syn1 = NULL;
    return nowy;
}


// Pomocnicza funkcja do zamiany elementow w tablicy
void zamien_miejscami(element_t **a, element_t **b) {
    element_t *temp = *a;
    *a = *b;
    *b = temp;
}

// Naprawianie kopca w dol
void sortuj_w_dol(priorytetowa_kolejka_t *k, int index) {
    int rozmiar = k->rozmiar_aktualny;
    int najmniejszy = index;

    while (1) {
        int l = 2 * index + 1; // lewy syn
        int p = 2 * index + 2; // prawy syn

        // Sprawdzam czy lewy syn jest mniejszy od obecnego
        if (l < rozmiar && k->tablica_wskaznikow[l]->czestosc < k->tablica_wskaznikow[najmniejszy]->czestosc) {
            najmniejszy = l;
        }

        // Sprawdzam czy prawy syn jest mniejszy od obecnego najmniejszego
        if (p < rozmiar && k->tablica_wskaznikow[p]->czestosc < k->tablica_wskaznikow[najmniejszy]->czestosc) {
            najmniejszy = p;
        }

        // Jesli znalezlismy mniejszego syna, to zamieniamy i idziemy dalej
        if (najmniejszy != index) {
            zamien_miejscami(&k->tablica_wskaznikow[index], &k->tablica_wskaznikow[najmniejszy]);
            index = najmniejszy;
        } else {
            break; // Kopiec jest juz poprawny
        }
    }
}

// Naprawianie kopca w gore
void sortuj_w_gore(priorytetowa_kolejka_t *k, int index) {
    while (index > 0) {
        int rodzic = (index - 1) / 2;
        // Jesli rodzic ma wieksza wage niz my, to zamieniamy sie miejscami
        if (k->tablica_wskaznikow[rodzic]->czestosc > k->tablica_wskaznikow[index]->czestosc) {
            zamien_miejscami(&k->tablica_wskaznikow[index], &k->tablica_wskaznikow[rodzic]);
            index = rodzic;
        } else {
            break;
        }
    }
}

// Dodawanie nowego elementu do kolejki
void wstaw_do_kolejki(priorytetowa_kolejka_t *k, element_t *el) {
    k->tablica_wskaznikow[k->rozmiar_aktualny] = el;
    k->rozmiar_aktualny++;
    sortuj_w_gore(k, k->rozmiar_aktualny - 1);
}

// Pobieranie elementu o najmniejszej wadze
element_t* pobierz_minimum(priorytetowa_kolejka_t *k) {
    if (k->rozmiar_aktualny <= 0) return NULL;

    element_t *wynik = k->tablica_wskaznikow[0];
    k->rozmiar_aktualny--;
    k->tablica_wskaznikow[0] = k->tablica_wskaznikow[k->rozmiar_aktualny];

    sortuj_w_dol(k, 0);
    return wynik;
}

// zmiana wagi istniejacego elementu
void aktualizuj_wage(priorytetowa_kolejka_t *k, char znak, int nowa_waga) {
    for (int i = 0; i < k->rozmiar_aktualny; ++i) {
        if (k->tablica_wskaznikow[i]->litera == znak) {
            int stara_waga = k->tablica_wskaznikow[i]->czestosc;
            k->tablica_wskaznikow[i]->czestosc = nowa_waga;

            // Decyzja czy naprawiac w gore czy w dol
            if (nowa_waga < stara_waga) sortuj_w_gore(k, i);
            else sortuj_w_dol(k, i);
            return;
        }
    }
}

// budowanie kolejki z tablicy
void inicjalizuj_kolejke(priorytetowa_kolejka_t *k, char *znaki, int *wagi, int n) {
    k->rozmiar_aktualny = 0;
    for (int i = 0; i < n; i++) {
        k->tablica_wskaznikow[i] = utworz_element(znaki[i], wagi[i]);
    }
    k->rozmiar_aktualny = n;

    // Budowanie kopca - zaczynam od polowy tablicy idac w dol
    for (int i = (n / 2) - 1; i >= 0; i--) {
        sortuj_w_dol(k, i);
    }
}


// generowanie kodow binarnych dla lisci
void rekurencyjne_szukanie_kodow(element_t *wezel, char *obecny_kod, int dlugosc, char mapa_kodow[256][256]) {
    // Idziemy w lewo - dopisujemy '0'
    if (wezel->syn0) {
        obecny_kod[dlugosc] = '0';
        obecny_kod[dlugosc + 1] = '\0';
        rekurencyjne_szukanie_kodow(wezel->syn0, obecny_kod, dlugosc + 1, mapa_kodow);
    }

    // Idziemy w prawo - dopisujemy '1'
    if (wezel->syn1) {
        obecny_kod[dlugosc] = '1';
        obecny_kod[dlugosc + 1] = '\0';
        rekurencyjne_szukanie_kodow(wezel->syn1, obecny_kod, dlugosc + 1, mapa_kodow);
    }

    // Jestesmy w lisciu - zapisujemy znaleziony kod dla znaku
    if (!wezel->syn0 && !wezel->syn1) {
        strcpy(mapa_kodow[(unsigned char)wezel->litera], obecny_kod);
    }
}


void wykonaj_kompresje(char *nazwa_we, char *nazwa_wy) {
    FILE *plik_we = fopen(nazwa_we, "rb");
    if (!plik_we) {
        printf("Blad: Nie udalo sie otworzyc pliku zrodlowego.\n");
        return;
    }

    // Zliczam ile razy wystepuje kazdy znak
    int histogram[256] = {0};
    int c;
    long licznik_znakow = 0;

    while ((c = fgetc(plik_we)) != EOF) {
        histogram[c]++;
        licznik_znakow++;
    }
    rewind(plik_we); // Wracam na poczatek pliku

    if (licznik_znakow == 0) {
        fclose(plik_we);
        return;
    }

    // Inicjalizuje kolejke i dodaje znaki
    priorytetowa_kolejka_t kolejka;
    kolejka.tablica_wskaznikow = malloc(256 * sizeof(element_t*));
    kolejka.rozmiar_aktualny = 0;

    int unikalne_znaki = 0;
    for (int i = 0; i < 256; i++) {
        if (histogram[i] > 0) {
            wstaw_do_kolejki(&kolejka, utworz_element((char)i, histogram[i]));
            unikalne_znaki++;
        }
    }

    // Buduje drzewo Huffmana
    while (kolejka.rozmiar_aktualny > 1) {
        element_t *e1 = pobierz_minimum(&kolejka);
        element_t *e2 = pobierz_minimum(&kolejka);

        // Nowy wezel ma wage rowna sumie dzieci
        element_t *rodzic = utworz_element(0, e1->czestosc + e2->czestosc);
        rodzic->syn0 = e1;
        rodzic->syn1 = e2;

        wstaw_do_kolejki(&kolejka, rodzic);
    }
    // Ostatni element to korzen drzewa
    element_t *korzen = pobierz_minimum(&kolejka);

    // Generuje kody dla kazdego znaku
    char slownik[256][256];
    char bufor_roboczy[256];
    memset(slownik, 0, sizeof(slownik));

    rekurencyjne_szukanie_kodow(korzen, bufor_roboczy, 0, slownik);

    // Zapisuje plik wynikowy
    FILE *plik_wy = fopen(nazwa_wy, "wb");

    // Zapisuje ilosc wpisow w slowniku
    fwrite(&unikalne_znaki, sizeof(int), 1, plik_wy);
    // Zapisuje pary: znak -> waga
    for (int i = 0; i < 256; i++) {
        if (histogram[i] > 0) {
            char znak = (char)i;
            fwrite(&znak, 1, 1, plik_wy);
            fwrite(&histogram[i], sizeof(int), 1, plik_wy);
        }
    }
    // Zapisuje calkowita dlugosc oryginalnego pliku
    fwrite(&licznik_znakow, sizeof(long), 1, plik_wy);

    // ompresja wlasciwa (pakowanie bitow)
    unsigned char bajt_wyjsciowy = 0;
    int przesuniecie = 0;

    while ((c = fgetc(plik_we)) != EOF) {
        char *kod = slownik[c];
        for (int i = 0; kod[i] != '\0'; i++) {
            // Przesuwam bity w lewo
            bajt_wyjsciowy = bajt_wyjsciowy << 1;
            if (kod[i] == '1') {
                bajt_wyjsciowy = bajt_wyjsciowy | 1; // Ustawiam bit na 1
            }
            przesuniecie++;

            if (przesuniecie == 8) {
                fputc(bajt_wyjsciowy, plik_wy);
                bajt_wyjsciowy = 0;
                przesuniecie = 0;
            }
        }
    }

    if (przesuniecie > 0) {
        bajt_wyjsciowy <<= (8 - przesuniecie);
        fputc(bajt_wyjsciowy, plik_wy);
    }

    fclose(plik_we);
    fclose(plik_wy);
    free(kolejka.tablica_wskaznikow);
    printf("Zakonczono kompresje -> %s\n", nazwa_wy);
}

void wykonaj_dekompresje(char *nazwa_we, char *nazwa_wy) {
    FILE *plik_we = fopen(nazwa_we, "rb");
    if (!plik_we) {
        printf("Blad: Brak pliku wejsciowego.\n");
        return;
    }

    int liczba_wpisow;
    if (fread(&liczba_wpisow, sizeof(int), 1, plik_we) != 1) {
        fclose(plik_we);
        return;
    }

    priorytetowa_kolejka_t kolejka;
    kolejka.tablica_wskaznikow = malloc(512 * sizeof(element_t*));
    kolejka.rozmiar_aktualny = 0;

    for (int i = 0; i < liczba_wpisow; i++) {
        char z;
        int w;
        fread(&z, 1, 1, plik_we);
        fread(&w, sizeof(int), 1, plik_we);
        wstaw_do_kolejki(&kolejka, utworz_element(z, w));
    }

    long calkowita_dlugosc;
    fread(&calkowita_dlugosc, sizeof(long), 1, plik_we);

    while (kolejka.rozmiar_aktualny > 1) {
        element_t *l = pobierz_minimum(&kolejka);
        element_t *p = pobierz_minimum(&kolejka);
        element_t *nowy = utworz_element(0, l->czestosc + p->czestosc);
        nowy->syn0 = l;
        nowy->syn1 = p;
        wstaw_do_kolejki(&kolejka, nowy);
    }
    element_t *korzen = pobierz_minimum(&kolejka);

    // Dekodowanie danych
    FILE *plik_wy = fopen(nazwa_wy, "wb");
    element_t *obecny = korzen;
    int bajt;
    long zapisane = 0;

    while ((bajt = fgetc(plik_we)) != EOF && zapisane < calkowita_dlugosc) {
        for (int i = 7; i >= 0; i--) {
            if (zapisane >= calkowita_dlugosc) break;

            int bit = (bajt >> i) & 1;

            if (bit == 0) obecny = obecny->syn0; // Ide w lewo
            else obecny = obecny->syn1;          // Ide w prawo

            if (obecny->syn0 == NULL && obecny->syn1 == NULL) {
                fputc(obecny->litera, plik_wy);
                zapisane++;
                obecny = korzen;
            }
        }
    }

    fclose(plik_we);
    fclose(plik_wy);
    free(kolejka.tablica_wskaznikow);
    printf("Zakonczono dekompresje -> %s\n", nazwa_wy);
}


int main() {
    char sciezka_we[100];
    char sciezka_wy[100];
    int tryb_pracy;

    while (1) {
        printf("\n1. Kompresuj plik\n2. Dekompresuj plik\n0. Wyjdz\nWybierz opcje: ");
        if (scanf("%d", &tryb_pracy) != 1) break;

        if (tryb_pracy == 0) break;

        if (tryb_pracy == 1) {
            printf("Podaj plik zrodlowy: ");
            scanf("%s", sciezka_we);
            printf("Podaj plik wynikowy: ");
            scanf("%s", sciezka_wy);
            wykonaj_kompresje(sciezka_we, sciezka_wy);
        } else if (tryb_pracy == 2) {
            printf("Podaj plik skompresowany: ");
            scanf("%s", sciezka_we);
            printf("Podaj plik wynikowy: ");
            scanf("%s", sciezka_wy);
            wykonaj_dekompresje(sciezka_we, sciezka_wy);
        } else {
            printf("Zla opcja.\n");
        }
    }

    return 0;
}