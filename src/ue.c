#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1" // Assuming the server is running on localhost

volatile int sfn = 0;

// Function to update SFN
void* update_sfn() {
    while (1) {
        sleep(1); // Wait for 1 second
        sfn = (sfn + 1) % 1024; // Increment SFN and reset after reaching 1023
        printf("[UE SFN: %d]\n", sfn);
    }
}

// Function to act as UDP client
void* udp_client() {
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (1) {
        if (sfn % 5 == 0) {
            // Send a ping to the server every 10 seconds
        const char* message = "Ping from UE";
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
        printf("Ping sent to gNB\n");

        // Optionally, receive a response from the server
        char buffer[1024];
        struct sockaddr_in from;
        socklen_t fromLen = sizeof(from);
        int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&from, &fromLen);
        buffer[n] = '\0';
        printf("Server: %s\n", buffer);
        sleep(1);
        }
    }

    close(sockfd);
}

int main() {
    pthread_t thread1, thread2;

    // Creating two threads, one for updating SFN and another for UDP client
    if (pthread_create(&thread1, NULL, update_sfn, NULL) != 0) {
        perror("Failed to create thread for updating SFN");
        return 1;
    }

    if (pthread_create(&thread2, NULL, udp_client, NULL) != 0) {
        perror("Failed to create thread for UDP client");
        return 1;
    }

    // Joining threads with the main thread
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}