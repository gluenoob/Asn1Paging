#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile int sfn = 0; // System Frame Number counter

void *update_sfn(void *arg)
{
    while (1)
    {
        sleep(1);
        sfn = (sfn + 1) % 1024;
    }
}

void *udp_client(void *arg)
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while (1)
    {
        char buffer[1024];
        int len = recv(sockfd, buffer, sizeof(buffer), 0);
        if (len > 0)
        {
            // Decode Paging message using ASN.1 decoding functions
            printf("Received message\n"); // Placeholder for demonstration
        }
    }
}

int main()
{
    pthread_t threads[2];

    pthread_create(&threads[0], NULL, update_sfn, NULL);
    pthread_create(&threads[1], NULL, udp_client, NULL);

    for (int i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}