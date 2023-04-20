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

    printf("Client %d says bye!!!\n\n", getpid());

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

    // Пытаемся получить дескриптор общей памяти
    if ((shm_pid = shmget(shm_key, mem_pid_size, 0666)) == -1)
    {
        printf("Hairdresser's is closed\n");
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
    if ((chair_sem = semget(chair_sem_key, 1, 0666)) < 0)
    {
        printf("Hairdresser's is closed\n");
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
    if ((hairdresser_sem = semget(hairdresser_sem_key, 1, 0666)) < 0)
    {
        printf("Hairdresser's is closed\n");
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

    printf("Creator pid = %d, ppid = %d\n\n", getpid(), getppid());

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

    semop(chair_sem, &mybuf_wait, 1); // chair-
    printf("%d sat in the chair\n", getpid());
    addr_pid[0] = getpid();
    semop(hairdresser_sem, &mybuf_post, 1); // hairdresser+

    return 0;
}
