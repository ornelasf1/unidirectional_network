#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_mutex_t bsem;
static char direction[10];

void *channel_thread(void *family_void_ptr)
{
    int i = 0;
    while (1)
    {
        pthread_mutex_lock(&bsem);
        switch (i)
        {
        case 0:
            strcpy(direction, "AB");
            break;
        case 1:
            strcpy(direction, "NONE");
            break;
        case 2:
            strcpy(direction, "BA");
            break;
        case 3:
            strcpy(direction, "NONE");
            break;
        }
        printf("The current direction is %s\n", direction);
        pthread_mutex_unlock(&bsem);
        sleep(5);
        i = (i + 1) % 4;
    }
    return NULL;
}

int main()
{

    pthread_t tid;
    static int members = 0;
    pthread_mutex_init(&bsem, NULL); // Initialize access to 1

    if (pthread_create(&tid, NULL, channel_thread, (void *)NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    printf("Finished condition\n");
    // Wait for the other threads to finish.
    pthread_join(tid, NULL);
    printf("Here is after\n");
    return 0;
}