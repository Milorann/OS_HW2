#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int chair_sem;

int hairdresser_sem;

struct sembuf mybuf_post;
struct sembuf mybuf_wait;

int *addr_pid;
int shm_pid;
int mem_pid_size = sizeof(int);

void sigfunc(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
    {
        return;
    }

    if (semctl(chair_sem, 0, IPC_RMID, 0) < 0)
    {
        printf("Couldn't close chair_sem\n");
    }

    if (semctl(hairdresser_sem, 0, IPC_RMID, 0) < 0)
    {
        printf("Couldn't close hairdresser_sem\n");
    }

    // Удаляем область
    if (shmctl(shm_pid, IPC_RMID, NULL) < 0)
    {
        printf("Couldn't close hairdresser_sem\n");
    }

    printf("Hairdresser %d says bye!!!\n\n", getpid());

    exit(10);
}

void init()
{
    key_t shm_key;
    if ((shm_key = ftok("8-hairdresser.c", 0)) < 0)
    {
        printf("Couldn't generate shm_key\n");
        exit(-1);
    }

    // Пытаемся получить дескриптор общей памяти, если не создана, то создать
    if ((shm_pid = shmget(shm_key, mem_pid_size, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        printf("Shared memory opening error\n");
        exit(-1);
    }
    // Устанавливаем размер области
    if ((addr_pid = (int *)shmat(shm_pid, NULL, 0)) == (int *)(-1))
    {
        printf("Couldn't attach shared memory\n");
        exit(-1);
    }

    mybuf_post.sem_num = 0;
    mybuf_post.sem_flg = 0;
    mybuf_post.sem_op = 1;

    mybuf_wait.sem_num = 0;
    mybuf_wait.sem_flg = 0;
    mybuf_wait.sem_op = -1;

    key_t chair_sem_key;
    if ((chair_sem_key = ftok("8-hairdresser.c", 1)) < 0)
    {
        printf("Can\'t generate chair_sem_key\n");
        exit(-1);
    }
    if ((chair_sem = semget(chair_sem_key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0)
    {
        printf("Can\'t connect to chair_sem\n");
        exit(-1);
    }
    else
    {
        semctl(chair_sem, 0, SETVAL, 0);
    }

    key_t hairdresser_sem_key;
    if ((hairdresser_sem_key = ftok("8-hairdresser.c", 2)) < 0)
    {
        printf("Can\'t generate hairdresser key\n");
        exit(-1);
    }
    if ((hairdresser_sem = semget(hairdresser_sem_key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0)
    {
        printf("Can\'t connect to hairdressser_sem\n");
        exit(-1);
    }
    else
    {
        semctl(hairdresser_sem, 0, SETVAL, 0);
    }
}

int main()
{
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);

    srand(time(NULL));

    init();

    // Haidresser
    printf("Hairdresser pid = %d, ppid = %d\n", getpid(), getppid());
    printf("Haidresser %d started working\n", getpid());
    int gv;
    semop(chair_sem, &mybuf_post, 1); // chair

    while (1)
    {
        semop(hairdresser_sem, &mybuf_wait, 1); // hairdresser
        printf("Haidresser got client %d\n", addr_pid[0]);
        sleep(5);
        printf("Client %d got haircut\n", addr_pid[0]);
        printf("%d leaves\n\n", addr_pid[0]);
        printf("Hairdresser goes to sleep\n");
        semop(chair_sem, &mybuf_post, 1); // chair+
    }

    return 0;
}
