// Created by: Leonard-Gabriel Necula
// Date: 10.04.2021
//
// Modified: 16.04.2021
//
// Programarea Aplicatiilor in Timp Real

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

sem_t Sem[3];

void task1(); // umplere sticla
void task2(); // adaugare dop
void task3(); // oprire banda transportoare

void (*tasks[])() = {task1, task2, task3};
void status_check(); // verificare status banda

// Constante de timp in [secunde]
int t_umplere = 2;
int t_dop = 5;
int t_detectare = 15;

// stare banda, 1 - banda pornita
// 0 - banda oprita
int banda = 1;

// Mecanism de excludere mutuala pentru accesarea
// zonei de memorie cu statusul benzii transportoare
pthread_mutex_t mutex;

int main(int argc, char **argv)
{

    pthread_t FIR[3];    // definire fire de executie
    pthread_attr_t attr; // definire atribute fire

    timer_t timerid;           // definire timer
    struct sigevent sev;       // definire ss
    struct itimerspec trigger; // definire structura timp
                               // pentru timer
    struct sigaction act;      // actiuni la aparitia semnalelor
    sigset_t set;              // set de semnale tratate

    sigemptyset(&set);        // golire set semnale
    sigaddset(&set, SIGUSR1); // adaugare semnal

    // Configurare actiune la aparitia unui semnal
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &status_check;
    // Configurare actiune -> semnal
    sigaction(SIGUSR1, &act, NULL);

    memset(&sev, 0, sizeof(struct sigevent));
    memset(&trigger, 0, sizeof(struct itimerspec));

    // Setare task ciclic generat de timer
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = &task3;

    // Setare timer
    timer_create(CLOCK_REALTIME, &sev, &timerid);
    trigger.it_value.tv_sec = t_detectare;
    trigger.it_interval.tv_sec = t_detectare;
    timer_settime(timerid, 0, &trigger, NULL);

    if ((pthread_mutex_init(&mutex, NULL)))
    {
        perror("pthread_mutex_init() failed");
        return EXIT_FAILURE;
    }

    int i;

    // Initializare semafoare generalizate
    for (i = 0; i < 3; i++)
        if (!sem_init(Sem + i, 1, 0))
        {
            ; // Nu facem nimic in cazul in care initializarea a fost
              // facuta cu succes
        }
        else
            printf("Eroare la initializarea semaforului numarul %d \n", i + 1);

    // Creare fire de executie
    for (i = 0; i < 3; i++)
        if (pthread_create(FIR + i, &attr, (void *)(*(tasks + i)), NULL) != 0)
        {
            perror("pthread_create");
            return EXIT_FAILURE;
        }

    while (1)
        ;

    return 0;
}

void task1()
{
    while (1)
    {
        sem_wait(Sem + 0); // P(S0)
        fflush(stdout);
        puts("Umplere sticla - START");
        sleep(t_umplere);
        puts("Umplere sticla - FINISH");
        sem_post(Sem + 2); // V(S2)
    }
}

void task2()
{
    while (1)
    {
        sem_wait(Sem + 1); // P(S1)
        fflush(stdout);
        puts("Adaugare dop - START");
        sleep(t_dop);
        puts("Adaugare dop - FINISH");
        sem_post(Sem + 2); // V(S2)
    }
}

void task3()
{
    pthread_mutex_lock(&mutex);
    banda = 0;
    pthread_mutex_unlock(&mutex);
    sem_post(Sem + 0); // V(S0)
    sem_post(Sem + 1); // V(S1)
    sem_wait(Sem + 2); // P(S2)
    sem_wait(Sem + 2); // P(S2)
    pthread_mutex_lock(&mutex);
    banda = 1;
    pthread_mutex_unlock(&mutex);
}

void status_check()
{
    fflush(stdout);
    pthread_mutex_lock(&mutex);
    printf("\n###### Banda = %d ######\n", banda);
    pthread_mutex_unlock(&mutex);
}