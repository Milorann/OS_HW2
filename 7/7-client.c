#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

const char *chair_sem_name = "/chair-semaphore";
sem_t *chair_sem;

const char *hairdresser_sem_name = "/hairdresser-semaphore";
sem_t *hairdresser_sem;

char memn_pid[] = "shared-memory-posix-4-pid";

int *addr_pid;
int shm_pid;
int mem_pid_size = sizeof(int);

void sigfunc(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
    {
        return;
    }

    printf("Client %d says bye!!!\n\n", getpid());

    exit(10);
}

void init()
{
    // Пытаемся получить дескриптор общей памяти, если не создана, то уходим
    if ((shm_pid = shm_open(memn_pid, O_RDWR, 0666)) == -1)
    {
        printf("Hairdresser's is closed\n");
        exit(-1);
    }

    // Получаем доступ к памяти
    addr_pid = (int *)mmap(0, mem_pid_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_pid, 0);
    if (addr_pid == (int *)-1)
    {
        printf("Error getting pointer to shared memory\n");
        perror("mmap");
        exit(-1);
    }

    if ((chair_sem = sem_open(chair_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("sem_open: Can not create chair semaphore");
        exit(-1);
    };

    if ((hairdresser_sem = sem_open(hairdresser_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("sem_open: Can not create hairdresser semaphore");
        exit(-1);
    };
}

int main()
{
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);

    srand(time(NULL));

    init();

    printf("Client %d came to hairdresser's and got in line\n", getpid());

    sem_wait(chair_sem); // chair-
    printf("%d sat in the chair\n", getpid());
    addr_pid[0] = getpid();
    sem_post(hairdresser_sem); // hairdresser+

    return 0;
}
