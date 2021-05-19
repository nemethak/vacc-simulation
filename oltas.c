#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    //sleep, fork
#include <sys/types.h> //fork, ftok
#include <wait.h>
#include <signal.h>
#include <time.h>

#define HARCRA_FEL SIGUSR1

struct Patient
{
    char name[100];
    int birthYear;
    char phoneNumber[20];
    int paying;
    char vaccinated[10];
};

void readPatient(struct Patient *patient)
{
    char name[100];
    int year;
    char phone[20];
    int paying;
    char vaccinated[10];

    printf("Kérem adja meg az adatokat!\nNév: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    printf("Született: ");
    scanf("%d", &year);
    printf("Telefonszám: ");
    scanf("%s", phone);
    printf("Vállalja a fizetős felárat? (0: nem, 1: igen): ");
    scanf("%d", &paying);

    strcpy(patient->name, name);
    patient->birthYear = year;
    strcpy(patient->phoneNumber, phone);
    patient->paying = paying;
    strcpy(patient->vaccinated, "VÁR");
}

int modifyPatient(char *file, char *name, int mode) // mode 1: törlés, minden más módosítás
{
    FILE *f;
    FILE *f_tmp;
    int found = 0;
    struct Patient patient;

    f = fopen(file, "rb");
    if (!f)
    {
        printf("\nHiba a fájl megnyitásakor: %s\n", file);
        return -1;
    }
    f_tmp = fopen("tmp.txt", "wb");
    if (!f_tmp)
    {
        printf("Hiba az ideiglenes állomány létrehozásakor.");
        return -1;
    }

    while (fread(&patient, sizeof(struct Patient), 1, f))
    {
        if (strcmp(name, patient.name) == 0)
        {
            if (mode == 1)
            {
                printf("A páciens adatai törlődtek.\n");
            }
            else
            {
                printf("Ön a %s nevű páciens adatait módosítja.\n", name);
                if (found == 1)
                    fgetc(stdin);
                readPatient(&patient);
                fwrite(&patient, sizeof(struct Patient), 1, f_tmp);
                printf("Sikeres módosítás.\n");
            }
            found = 1;
        }
        else
        {
            fwrite(&patient, sizeof(struct Patient), 1, f_tmp);
        }
    }
    if (!found)
    {
        printf("A megadott névvel nem található páciens: %s\n", name);
    }

    fclose(f);
    fclose(f_tmp);

    remove(file);
    rename("tmp.txt", file);

    return 0;
}

int vaccinatePatient(char *file, char *name)
{
    FILE *f;
    FILE *f_tmp;
    struct Patient patient;

    f = fopen(file, "rb");
    if (!f)
    {
        printf("\nHiba a fájl megnyitásakor: %s\n", file);
        return -1;
    }
    f_tmp = fopen("tmp.txt", "wb");
    if (!f_tmp)
    {
        printf("Hiba az ideiglenes állomány létrehozásakor.");
        return -1;
    }

    while (fread(&patient, sizeof(struct Patient), 1, f))
    {
        if (strcmp(name, patient.name) == 0)
        {
            strcpy(patient.vaccinated, "OLTVA");
            fwrite(&patient, sizeof(struct Patient), 1, f_tmp);
        }
        else
        {
            fwrite(&patient, sizeof(struct Patient), 1, f_tmp);
        }
    }

    fclose(f);
    fclose(f_tmp);

    remove(file);
    rename("tmp.txt", file);

    return 0;
}

int countWaiting(char *file)
{
    FILE *f;
    int waiting = 0;
    struct Patient patient;

    f = fopen(file, "r");
    if (!f)
    {
        printf("\nHiba a fájl megnyitásakor: %s\n", file);
        return -1;
    }

    while (fread(&patient, sizeof(struct Patient), 1, f))
    {
        if (strcmp("VÁR", patient.vaccinated) == 0)
        {
            waiting++;
        }
    }
    fclose(f);
    return waiting;
}

int send(pid_t target, int signal)
{
    return kill(target, signal);
}

void handler(int signal)
{
    printf("%d folyamat %d signalt kapott.\n", getpid(), signal);
}

void printMenu()
{
    int choice;
    FILE *data;

    do
    {
        printf("\nMenü\n\n");
        printf("1. Adatfelvétel\n");
        printf("2. Adatmódosítás\n");
        printf("3. Adatok törlése\n");
        printf("4. Listázás\n");
        printf("5. Kilépés\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1: //adatfelvétel
        {
            struct Patient patient;

            fgetc(stdin);
            readPatient(&patient);

            data = fopen("data2.txt", "a");
            if (!data)
            {
                printf("\nHiba a fájl megnyitásakor.\n");
                exit(1);
            }

            fwrite(&patient, sizeof(struct Patient), 1, data);
            if (fwrite != 0)
                printf("Sikeres adatfelvétel!\n");
            else
                printf("Hiba a fájlba íráskor!\n");
            fclose(data);
            break;
        }
        case 2: //adatmódosítás
        {
            char name[100];
            fgetc(stdin);
            printf("Kérem adja meg a módosítani kívánt páciens nevét: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;
            modifyPatient("data2.txt", name, 0);
            break;
        }
        case 3: //törlés
        {
            char name[100];
            fgetc(stdin);
            printf("Kérem adja meg a törölni kívánt páciens nevét: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;
            modifyPatient("data2.txt", name, 1);
            break;
        }
        case 4: //listázás
        {
            struct Patient patient;

            data = fopen("data2.txt", "r");
            if (!data)
            {
                printf("\nHiba a fájl megnyitásakor.\n");
                exit(1);
            }
            while (fread(&patient, sizeof(struct Patient), 1, data))
            {
                char *tmp = (patient.paying == 1) ? "igen" : "nem";
                printf("Név: %s\tSzületett: %d\tTelefonszám: %s\tFizető: %s\t %s\n", patient.name, patient.birthYear, patient.phoneNumber, tmp, patient.vaccinated);
            }
            fclose(data);
            break;
        }
        case 5: //kilépés
            break;
        default:
            printf("Érvénytelen választás, válasszon ismét.\n");
            break;
        }
    } while (choice != 5);
}

void childReadPatients(struct Patient *patients, int offset)
{
    for (int i = 0; i < 5; i++)
    {
        char *tmp = (patients[i].paying == 1) ? "igen" : "nem";
        printf("Név: %s\tSzületett: %d\tTelefonszám: %s\tFizető: %s\t %s\n", patients[i].name, patients[i].birthYear, patients[i].phoneNumber, tmp, patients[i].vaccinated);
        int random = abs(rand() * offset) % 100;
        //printf("random: %d\n", random);
        if (random <= 90)
        {
            strcpy(patients[i].vaccinated, "OLTVA");
            //printf("%s\n", patients[i].name);
        }
    }
}

void markVaccinated(struct Patient *patients)
{
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(patients[i].vaccinated, "OLTVA") == 0)
        {
            vaccinatePatient("data2.txt", patients[i].name);
        }
    }
}

void writePipe(int pipefd[2], struct Patient *patients)
{
    close(pipefd[0]);
    write(pipefd[1], patients, sizeof(struct Patient) * 5);
    close(pipefd[1]);
}

void readPipe(int pipefd[2], struct Patient *patients)
{
    close(pipefd[1]);
    read(pipefd[0], patients, sizeof(struct Patient) * 5);
    close(pipefd[0]);
}

int setChildrenNeeded(char *file)
{
    if (countWaiting(file) > 4 && countWaiting(file) < 10)
    {
        return 1;
    }
    else if (countWaiting(file) > 9)
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

int main()
{
    srand(time(NULL));

    int choice;

    printf("\nMenü\n\n");
    printf("1. Adatkezelés\n");
    printf("2. Oltás\n");
    printf("3. Kilépés\n");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
    {
        printMenu();
        break;
    }
    case 2:
    {
        int childrenNeeded = setChildrenNeeded("data2.txt");

        pid_t child, child2;
        struct Patient patient;
        FILE *data;
#pragma region pipes
        int pipefd[2];
        int pipefd2[2];
        int pipefd3[2];
        int pipefd4[2];
        int pipefd5[2];
        int pipefd6[2];

        if (pipe(pipefd) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }

        if (pipe(pipefd2) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }

        if (pipe(pipefd3) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }

        if (pipe(pipefd4) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }

        if (pipe(pipefd5) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }

        if (pipe(pipefd6) == -1)
        {
            perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
        }
#pragma endregion

        if (childrenNeeded == 1)
        {
            child = fork();
            if (child < 0)
            {
                perror("Hiba a villázáskor.\n");
                exit(1);
            }
            if (child > 0) //parent
            {
                //TODO: fogadta a jelzést, veszi a sorrendben következő oltásra várók adatait
                signal(HARCRA_FEL, handler);
                pause();

                //TODO: konzolra írja, hogy mely felhasználókat várja a megfelelő oltóbusz
                data = fopen("data2.txt", "r");
                if (!data)
                {
                    printf("\nHiba a fájl megnyitásakor.\n");
                    exit(1);
                }

                int counter = 0;
                struct Patient patients[5];
                while (counter < 5)
                {
                    fread(&patient, sizeof(struct Patient), 1, data);
                    {
                        if (strcmp(patient.vaccinated, "VÁR") == 0)
                        {
                            printf("1. oltóbusz várja: %s\n", patient.name);
                            patients[counter] = patient;
                            counter++;
                        }
                    }
                }
                fclose(data);

                //TODO: csővezetéken elküldi a páciensek adatait az oltóbusznak
                writePipe(pipefd, patients);
                wait(NULL);

                //TODO: kiket sikerült beoltani  kiolvassa, bejegyzi az "OLTVA" bejegyzést az adatállományba
                readPipe(pipefd2, patients);
                markVaccinated(patients);
            }
            else if (child == 0) //child
            {
                //TODO: "HARCRA_FEL" jelzést küld(enek) a Törzsnek
                printf("Oltóbusz 1. %d process id-n, a szülő a %d  id.\n", getpid(), getppid());
                send(getppid(), HARCRA_FEL);

                //TODO: páciensek adatait kiolvassa és nyugtázásul a képernyőre is írja azt.
                struct Patient patients[5];
                readPipe(pipefd, patients);

                //TODO: A páciensek 90% valószínűséggel megérkeznek az oltópontra(oltóbuszra), ahol megkapják az oltást.
                childReadPatients(patients, getpid());

                //TODO: visszaírja csővezetéken a Törzsnek, hogy kiket sikerült beoltani
                writePipe(pipefd2, patients);
            }
        }
        else if (childrenNeeded == 2)
        {
            child = fork();
            if (child < 0)
            {
                perror("Hiba a villázáskor.\n");
                exit(1);
            }
            if (child > 0)
            {
                child2 = fork();
                if (child2 < 0)
                {
                    perror("Hiba a villázáskor.\n");
                    exit(1);
                }
                else if (child2 > 0) // parent
                {
                    //TODO: fogadta a jelzést, veszi a sorrendben következő oltásra várók adatait
                    signal(HARCRA_FEL, handler);
                    pause();
                    pause();

                    //TODO: konzolra írja, hogy mely felhasználókat várja a megfelelő oltóbusz
                    data = fopen("data2.txt", "r");
                    if (!data)
                    {
                        printf("\nHiba a fájl megnyitásakor.\n");
                        exit(1);
                    }

                    int counter = 0;
                    struct Patient patients1[5];
                    struct Patient patients2[5];
                    while (counter < 10)
                    {
                        fread(&patient, sizeof(struct Patient), 1, data);
                        {
                            if (strcmp(patient.vaccinated, "VÁR") == 0)
                            {
                                if (counter < 5)
                                {
                                    printf("1. oltóbusz várja: %s\n", patient.name);
                                    patients1[counter] = patient;
                                }
                                else
                                {
                                    printf("2. oltóbusz várja: %s\n", patient.name);
                                    patients2[counter - 5] = patient;
                                }
                                counter++;
                            }
                        }
                    }
                    fclose(data);

                    //TODO: csővezetéken elküldi a páciensek adatait az oltóbusznak
                    writePipe(pipefd3, patients1);
                    writePipe(pipefd4, patients2);
                    wait(NULL);

                    //TODO: kiket sikerült beoltani kiolvassa, bejegyzi az "OLTVA" bejegyzést az adatállományba.
                    readPipe(pipefd5, patients1);
                    markVaccinated(patients1);
                    readPipe(pipefd6, patients2);
                    markVaccinated(patients2);
                }
                else if (child2 == 0) //child2
                {
                    struct Patient patients2[5];
                    //TODO: "HARCRA_FEL" jelzést küld(enek) a Törzsnek
                    printf("Oltóbusz 2. %d process id-n, a szülő a %d  id.\n", getpid(), getppid());
                    send(getppid(), HARCRA_FEL);

                    //TODO: páciensek adatait kiolvassa és nyugtázásul a képernyőre is írja azt
                    //TODO: A páciensek 90% valószínűséggel megérkeznek az oltópontra(oltóbuszra), ahol megkapják az oltást.
                    readPipe(pipefd4, patients2);
                    childReadPatients(patients2, getpid());

                    //TODO: visszaírja csővezetéken a Törzsnek, hogy kiket sikerült beoltani
                    writePipe(pipefd6, patients2);
                }
            }
            else if (child == 0) //child
            {
                
                sleep(1);
                struct Patient patients1[5];

                //TODO: "HARCRA_FEL" jelzést küld(enek) a Törzsnek
                printf("Oltóbusz 1. %d process id-n, a szülő a %d  id.\n", getpid(), getppid());
                send(getppid(), HARCRA_FEL);

                //TODO: páciensek adatait kiolvassa és nyugtázásul a képernyőre is írja azt.
                //TODO: A páciensek 90% valószínűséggel megérkeznek az oltópontra(oltóbuszra), ahol megkapják az oltást.
                readPipe(pipefd3, patients1);
                childReadPatients(patients1, getpid());

                //TODO: visszaírja csővezetéken a Törzsnek, hogy kiket sikerült beoltani
                writePipe(pipefd5, patients1);
            }
        }
        else
        {
            printf("Nincs elég jelentkező.\n");
        }
        break;
    }
    case 3: //kilépés
        break;
    default:
    {
        printf("Érvénytelen választás.\n");
    }
    }
    return 0;
};