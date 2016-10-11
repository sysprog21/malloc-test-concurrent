#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

static void *membench(void *arg);

typedef struct {
    int alloc_size;
    int count;
    int loop;
    int verbose;
} thread_args_t;

int main(int argc, char **argv)
{
    int c;
    int thread_num = 1;

    thread_args_t thread_args = {
        .alloc_size = 1,
        .count = 1,
        .loop = 1,
        .verbose = false,
    };

    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"size", required_argument, 0, 's'},
        {"count", required_argument, 0, 'c'},
        {"loop", required_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((c = getopt_long(argc, argv, "t:s:c:l:v",
                            long_options, &option_index)) != -1) {
        switch (c) {
        case 't':
            thread_num = atoi(optarg);
            break;
        case 's':
            thread_args.alloc_size = atoi(optarg);
            break;
        case 'c':
            thread_args.count = atoi(optarg);
            break;
        case 'l':
            thread_args.loop = atoi(optarg);
            break;
        case 'v':
            thread_args.verbose = true;
            break;
        case ':':
        case '?':
            fprintf(stderr,
                    "Usage: %s --thread=THREADNUMBER --size=MALLOCSIZE "
                    "--count=MALLOCCOUNT --loop=LOOPCOUNT\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    pthread_t p[thread_num];
    struct timeval t0, t1;

    printf("Number of threads : %d\n", thread_num);
    printf("Malloc size : %d MB\n", thread_args.alloc_size);
    printf("Number of councurrent malloc per thread: %d\n", thread_args.count);
    printf("Number of loops per thread : %d\n", thread_args.loop);

    gettimeofday(&t0, NULL);

    for (int i = 0; i < thread_num; i++)
        pthread_create(&p[i], NULL, membench, &thread_args);

    for (int i = 0; i < thread_num; i++)
        pthread_join(p[i], NULL);

    gettimeofday(&t1, NULL);

    /* get elapsed time */
    t1.tv_sec -= t0.tv_sec;
    if (t1.tv_usec < t0.tv_usec) {
        t1.tv_sec -=1;
        t1.tv_usec += 1000000 - t0.tv_usec;
    } else {
        t1.tv_usec -= t0.tv_usec;
    }

    printf("%d.%06d sec\n", (int) t1.tv_sec, (int) t1.tv_usec);

    exit(EXIT_SUCCESS);
}

static pid_t gettid(void)
{
    return syscall(SYS_gettid);
}

/* memory benchmark */
static void *membench(void *arg)
{
    thread_args_t *thread_args = (thread_args_t *) arg;

    int alloc_size = thread_args->alloc_size;
    int count = thread_args->count;
    int loop = thread_args->loop;
    int verbose = thread_args->verbose;

    int *mem[count];
    int size[count];

    int loopcnt = 0;

    while (1) {
        if (loop != 0 && ++loopcnt > loop)
            break;

        for (int i = 0; i < count; i++) {
            size[i] = (rand() % (alloc_size * 10) + 1) * 100 * 1000;
            mem[i]  = (int *) malloc( size[i] );

            if (!mem[i]) {
                puts("Fail to allocate memory.");
                return NULL;
            }
            memset(mem[i], 1, size[i]);
            memset(mem[i], 0, size[i]);
            if (verbose) {
                printf("thread: %d, loop: %d, mem[%d] %d Bytes malloc\n",
                       gettid(), loopcnt, i, size[i]);
            }
        }

        for (int i = 0; i < count; i++) {
            free(mem[i]);
            if (verbose) {
                printf("thread: %d, loop: %d, mem[%d] %d Bytes free\n",
                       gettid(), loopcnt, i, size[i]);
            }
        }
    }
    return NULL;
}
