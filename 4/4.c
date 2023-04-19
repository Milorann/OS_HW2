#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

int pid; // Родительский процесс парикмахера.
sem_t chair_sem;
sem_t hairdresser_sem;
char memn_pid[] = "shared-memory-posix-4-pid";
char memn_sems[] = "shared-memory-posix-4-sems";

int *addr_pid;
int shm_pid;
int mem_pid_size = sizeof(int);

sem_t *addr_sems;
int shm_sems;
int mem_sems_size = 2 * sizeof(sem_t);

// Функция, осуществляющая обработку сигнала прерывания работы
// Осществляет удаление всех семафоров и памяти. Заодно "убивает" читателя
// независимо от его текущего состояния
void sigfunc(int sig)
{
    if (sig != SIGINT)
    {
        return;
    }

    printf("The hairdresser's is closing\n");

    if (getpid() == pid)
    {

        if (sem_destroy(&chair_sem) < 0)
        {
            printf("Coildn't close chair_sem\n");
        }
        else
        {
            printf("Closed chair_sem\n");
        }

        if (sem_destroy(&hairdresser_sem) < 0)
        {
            printf("Coildn't close hairdresser_sem\n");
        }
        else
        {
            printf("Closed hairdresser_sem\n");
        }

        // Удаляем область
        if (shm_unlink(memn_pid) == -1)
        {
            printf("Shared memory pid is absent\n");
            // perror("shm_unlink");
        }
        // Удаляем область
        if (shm_unlink(memn_sems) == -1)
        {
            printf("Shared memory sems is absent\n");
            // perror("shm_unlink");
        }

        printf("Hairdresser %d says bye!!!\n\n", getpid());
    }
    else
    {
        printf("Client %d says bye!!!\n\n", getpid());
    }

    exit(10);
}

void init()
{
    // Пытаемся получить дескриптор общей памяти, если не создана, то создать
    if ((shm_pid = shm_open(memn_pid, O_CREAT | O_RDWR, 0666)) == -1)
    {
        printf("Opening error\n");
        perror("shm_open");
        exit(-1);
    }
    // Устанавливаем размер области
    ftruncate(shm_pid, mem_pid_size);
    // Получаем доступ к памяти
    addr_pid = (int *)mmap(0, mem_pid_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_pid, 0);
    if (addr_pid == (int *)-1)
    {
        printf("Error getting pointer to shared memory\n");
        perror("mmap");
        exit(-1);
    }

    // Пытаемся получить дескриптор общей памяти, если не создана, то создать
    if ((shm_sems = shm_open(memn_sems, O_CREAT | O_RDWR, 0666)) == -1)
    {
        printf("Opening error\n");
        perror("shm_open");
        exit(-1);
    }
    // Устанавливаем размер области
    ftruncate(shm_sems, mem_sems_size);
    // Получаем доступ к памяти
    addr_sems = (sem_t *)mmap(0, mem_sems_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_sems, 0);
    if (addr_sems == (sem_t *)-1)
    {
        printf("Error getting pointer to shared memory\n");
        perror("mmap");
        exit(-1);
    }

    if (sem_init(&chair_sem, 1, 0) < 0)
    {
        printf("Couldn't open chair_sem\n");
        exit(-1);
    }
    else
    {
        addr_sems[0] = chair_sem;
        printf("Opened chair_sem\n");
    }

    if (sem_init(&hairdresser_sem, 1, 0) < 0)
    {
        printf("Couldn't open hairdresser_sem\n");
        exit(-1);
    }
    else
    {
        addr_sems[1] = hairdresser_sem;
        printf("Opened hairdresser_sem\n");
    }
}

int main()
{
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);

    srand(time(NULL));

    init();

    pid = getpid(); // Родительский процесс парикмахера.

    fork();

    if (pid == getpid())
    { // Haidresser
        printf("Hairdresser pid = %d, ppid = %d\n", getpid(), getppid());
        printf("Haidresser %d started working\n", getpid());
        int gv;
        sem_post(&addr_sems[0]); // chair

        while (1)
        {
            sem_wait(&addr_sems[1]); // hairdresser
            printf("Haidresser got client %d\n", addr_pid[0]);
            sleep(5);
            printf("Client %d got haircut\n", addr_pid[0]);
            printf("%d released the chair\n", addr_pid[0]);
            kill(addr_pid[0], SIGINT);
            printf("Hairdresser goes to sleep\n");
            sem_post(&addr_sems[0]); // chair+
        }
    }
    else
    { // Clients creator
        printf("Creator pid = %d, ppid = %d\n", getpid(), getppid());

        while (1)
        { 
            // Clients come every 1 - 10 seconds
            sleep(rand() % 10 + 1);

            if (fork() == 0)
            {
                break;
            }
        }

        printf("Client %d came to hairdresser's and got in line\n", getpid());

        sem_wait(&addr_sems[0]); // chair-
        printf("%d sat in the chair\n", getpid());
        addr_pid[0] = getpid();
        sem_post(&addr_sems[1]); // hairdresser+
        sem_wait(&addr_sems[2]); // done-
    }

    return 0;
}
