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

    if (sem_unlink(chair_sem_name) < 0)
    {
        printf("Couldn't close chair_sem\n");
    }

    if (sem_unlink(hairdresser_sem_name) < 0)
    {
        printf("Couldn't close hairdresser_sem\n");
    }

    // Удаляем область
    if (shm_unlink(memn_pid) == -1)
    {
        printf("Shared memory pid is absent\n");
        // perror("shm_unlink");
    }

    printf("Hairdresser %d says bye!!!\n\n", getpid());

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

    printf("Hairdresser pid = %d, ppid = %d\n", getpid(), getppid());
    printf("Haidresser %d started working\n", getpid());
    int gv;
    sem_post(chair_sem); // chair

    while (1)
    {
        sem_wait(hairdresser_sem); // hairdresser
        printf("Haidresser got client %d\n", addr_pid[0]);
        sleep(5);
        printf("Client %d got haircut\n", addr_pid[0]);
        printf("%d leaves\n\n", addr_pid[0]);
        printf("Hairdresser goes to sleep\n");
        sem_post(chair_sem); // chair+
    }

    return 0;
}
