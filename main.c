#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Definim codurile de culoare ANSI
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_WHITE   "\033[1;37m"

#define MAX_JOCURI 20
#define MEMORIE_CONSOLA 1024.0

typedef struct {
    char nume[100];
    double dimensiune;
} Joc;

Joc jocuriInstalate[MAX_JOCURI];
int nrJocuriInstalate = 0;
double memorieDisponibila = MEMORIE_CONSOLA;

Joc jocuriDisponibile[MAX_JOCURI];
int nrJocuriDisponibile = 0;

const char* FISIER_MAGAZIN = "magazin_jocuri.txt";
const char* FISIER_INSTALATE = "jocuri_instalate.txt";

bool citesteJocDinLinie(const char* linie, Joc* joc) {
    if (linie[0] == '"') {
        const char* sfarsitNume = strchr(linie + 1, '"');
        if (!sfarsitNume) return false;
        size_t lungimeNume = sfarsitNume - (linie + 1);
        if (lungimeNume >= sizeof(joc->nume)) return false;
        strncpy(joc->nume, linie + 1, lungimeNume);
        joc->nume[lungimeNume] = '\0';
        const char* ptrDimensiune = sfarsitNume + 1;
        if (*ptrDimensiune != ',') return false;
        ptrDimensiune++;
        joc->dimensiune = atof(ptrDimensiune);
        return true;
    } else {
        char* virgula = strchr(linie, ',');
        if (!virgula) return false;
        size_t lungimeNume = virgula - linie;
        if (lungimeNume >= sizeof(joc->nume)) return false;
        strncpy(joc->nume, linie, lungimeNume);
        joc->nume[lungimeNume] = '\0';
        joc->dimensiune = atof(virgula + 1);
        return true;
    }
}

void incarcaJocuriDinMagazin() {
    FILE* fisier = fopen(FISIER_MAGAZIN, "r");
    if (fisier == NULL) {
        printf(COLOR_RED "Eroare: Fisierul magazin_jocuri.txt nu a putut fi deschis!\n" COLOR_RESET);
        exit(1);
    }
    nrJocuriDisponibile = 0;
    char linie[200];
    while (fgets(linie, sizeof(linie), fisier) != NULL && nrJocuriDisponibile < MAX_JOCURI) {
        linie[strcspn(linie, "\n")] = '\0';
        if (citesteJocDinLinie(linie, &jocuriDisponibile[nrJocuriDisponibile])) {
            nrJocuriDisponibile++;
        }
    }
    fclose(fisier);
}

void incarcaJocuriInstalate() {
    FILE* fisier = fopen(FISIER_INSTALATE, "r");
    if (fisier == NULL) return;
    nrJocuriInstalate = 0;
    memorieDisponibila = MEMORIE_CONSOLA;
    char linie[200];
    while (fgets(linie, sizeof(linie), fisier) != NULL && nrJocuriInstalate < MAX_JOCURI) {
        linie[strcspn(linie, "\n")] = '\0';
        if (citesteJocDinLinie(linie, &jocuriInstalate[nrJocuriInstalate])) {
            memorieDisponibila -= jocuriInstalate[nrJocuriInstalate].dimensiune;
            nrJocuriInstalate++;
        }
    }
    fclose(fisier);
}

void salveazaJocuriInstalate() {
    FILE* fisier = fopen(FISIER_INSTALATE, "w");
    if (fisier == NULL) {
        printf(COLOR_RED "Eroare la salvarea jocurilor instalate!\n" COLOR_RESET);
        return;
    }
    for (int i = 0; i < nrJocuriInstalate; i++) {
        if (strchr(jocuriInstalate[i].nume, ',') || strchr(jocuriInstalate[i].nume, '"')) {
            fprintf(fisier, "\"%s\",%.2f\n", jocuriInstalate[i].nume, jocuriInstalate[i].dimensiune);
        } else {
            fprintf(fisier, "%s,%.2f\n", jocuriInstalate[i].nume, jocuriInstalate[i].dimensiune);
        }
    }
    fclose(fisier);
}

void salveazaMagazin() {
    FILE* fisier = fopen(FISIER_MAGAZIN, "w");
    if (fisier == NULL) {
        printf(COLOR_RED "Eroare la salvarea magazinului!\n" COLOR_RESET);
        return;
    }
    for (int i = 0; i < nrJocuriDisponibile; i++) {
        if (strchr(jocuriDisponibile[i].nume, ',') || strchr(jocuriDisponibile[i].nume, '"')) {
            fprintf(fisier, "\"%s\",%.2f\n", jocuriDisponibile[i].nume, jocuriDisponibile[i].dimensiune);
        } else {
            fprintf(fisier, "%s,%.2f\n", jocuriDisponibile[i].nume, jocuriDisponibile[i].dimensiune);
        }
    }
    fclose(fisier);
}

void eliminaDinMagazin(int index) {
    if (index < 0 || index >= nrJocuriDisponibile) return;
    for (int i = index; i < nrJocuriDisponibile - 1; i++) {
        jocuriDisponibile[i] = jocuriDisponibile[i + 1];
    }
    nrJocuriDisponibile--;
    salveazaMagazin();
}

void adaugaInMagazin(Joc joc) {
    if (nrJocuriDisponibile < MAX_JOCURI) {
        jocuriDisponibile[nrJocuriDisponibile] = joc;
        nrJocuriDisponibile++;
        salveazaMagazin();
    }
}

int comparaDimensiune(const void *a, const void *b) {
    double diff = ((Joc *)a)->dimensiune - ((Joc *)b)->dimensiune;
    return (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
}

int comparaAlfabetic(const void *a, const void *b) {
    return strcmp(((Joc *)a)->nume, ((Joc *)b)->nume);
}

void sorteazaJocuriInstalate() {
    if (nrJocuriInstalate == 0) {
        printf(COLOR_YELLOW "Nu sunt jocuri instalate pentru sortare.\n" COLOR_RESET);
        return;
    }
    system("clear");
    int optiune;
    printf(COLOR_CYAN "1. Sorteaza dupa dimensiune (crescator)\n2. Sorteaza alfabetic\n" COLOR_RESET);
    printf("Alege o optiune: ");
    scanf("%d", &optiune);
    if (optiune == 1) {
        qsort(jocuriInstalate, nrJocuriInstalate, sizeof(Joc), comparaDimensiune);
    } else if (optiune == 2) {
        qsort(jocuriInstalate, nrJocuriInstalate, sizeof(Joc), comparaAlfabetic);
    } else {
        printf(COLOR_RED "Optiune invalida!\n" COLOR_RESET);
        return;
    }
    salveazaJocuriInstalate();
}

void vizualizeazaJocuriInstalate() {
    incarcaJocuriInstalate(); // <-- nou
    system("clear");
    if (nrJocuriInstalate == 0) {
        printf(COLOR_YELLOW "Nu sunt jocuri instalate.\n" COLOR_RESET);
        return;
    }
    printf(COLOR_GREEN "-= Jocuri instalate =-\n" COLOR_RESET);
    for (int i = 0; i < nrJocuriInstalate; i++) {
        printf("%d. " COLOR_CYAN "%s" COLOR_RESET " - " COLOR_YELLOW "%.2f GB\n" COLOR_RESET, i + 1, jocuriInstalate[i].nume, jocuriInstalate[i].dimensiune);
    }
    int choice;
    printf("\n0. Iesire\n100. Sorteaza jocurile\nAlege o optiune: ");
    scanf("%d", &choice);
    if (choice == 0) return;
    if (choice == 100) {
        sorteazaJocuriInstalate();
        vizualizeazaJocuriInstalate();
        return;
    }
    if (choice < 1 || choice > nrJocuriInstalate) {
        printf(COLOR_RED "Optiune invalida!\n" COLOR_RESET);
        return;
    }
    choice--;
    system("clear");
    printf(COLOR_CYAN "-= %s =-\n" COLOR_RESET, jocuriInstalate[choice].nume);
    printf("1. Dezinstaleaza\n2. Iesire\nAlege o optiune: ");
    int optiune;
    scanf("%d", &optiune);
    system("clear");
    if (optiune == 1) {
        printf(COLOR_GREEN "Ai dezinstalat cu succes jocul " COLOR_CYAN "%s" COLOR_GREEN "!\n" COLOR_RESET, jocuriInstalate[choice].nume);
        memorieDisponibila += jocuriInstalate[choice].dimensiune;
        adaugaInMagazin(jocuriInstalate[choice]);
        for (int i = choice; i < nrJocuriInstalate - 1; i++) {
            jocuriInstalate[i] = jocuriInstalate[i + 1];
        }
        nrJocuriInstalate--;
        salveazaJocuriInstalate();
    }
}

void instaleazaJoc() {
    incarcaJocuriDinMagazin(); // <-- nou
    system("clear");
    if (nrJocuriDisponibile == 0) {
        printf(COLOR_YELLOW "Nu sunt jocuri disponibile pentru instalare.\n" COLOR_RESET);
        return;
    }
    printf(COLOR_GREEN "-= Magazin =-\n" COLOR_RESET);
    for (int i = 0; i < nrJocuriDisponibile; i++) {
        printf("%d. " COLOR_CYAN "%s" COLOR_RESET " - " COLOR_YELLOW "%.2f GB\n" COLOR_RESET, i + 1, jocuriDisponibile[i].nume, jocuriDisponibile[i].dimensiune);
    }
    int choice;
    printf("\n0. Iesire\nAlege jocul de instalat (1-%d): ", nrJocuriDisponibile);
    scanf("%d", &choice);
    if (choice == 0) return;
    if (choice < 1 || choice > nrJocuriDisponibile) {
        printf(COLOR_RED "Optiune invalida!\n" COLOR_RESET);
        return;
    }
    choice--;
    system("clear");
    if (memorieDisponibila < jocuriDisponibile[choice].dimensiune) {
        printf(COLOR_RED "Nu exista suficient spatiu pe consola pentru acest joc.\n" COLOR_RESET);
    } else {
        jocuriInstalate[nrJocuriInstalate] = jocuriDisponibile[choice];
        nrJocuriInstalate++;
        memorieDisponibila -= jocuriDisponibile[choice].dimensiune;
        printf(COLOR_GREEN "Jocul " COLOR_CYAN "%s" COLOR_GREEN " a fost instalat cu succes!\n" COLOR_RESET, jocuriDisponibile[choice].nume);
        eliminaDinMagazin(choice);
        salveazaJocuriInstalate();
    }
}

void iesire() {
    system("clear");
    printf(COLOR_CYAN "La revedere!\n" COLOR_RESET);
    exit(0);
}

int main(void) {
    incarcaJocuriDinMagazin();
    incarcaJocuriInstalate();
    int choice;
    while (1) {
        system("clear");
        printf(COLOR_MAGENTA "\n-= Consola de jocuri =-\n" COLOR_RESET);
        printf(COLOR_YELLOW "Memorie disponibila: %.2f GB\n\n" COLOR_RESET, memorieDisponibila);
        printf(COLOR_BLUE "1. " COLOR_RESET "Jocuri instalate\n");
        printf(COLOR_BLUE "2. " COLOR_RESET "Magazin\n");
        printf(COLOR_BLUE "3. " COLOR_RESET "Iesire\n");
        printf(COLOR_WHITE "Introduceti o optiune: " COLOR_RESET);
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                vizualizeazaJocuriInstalate();
                break;
            case 2:
                instaleazaJoc();
                break;
            case 3:
                iesire();
                break;
            default:
                printf(COLOR_RED "Optiune invalida!\n" COLOR_RESET);
                break;
        }
    }
    return 0;
}
