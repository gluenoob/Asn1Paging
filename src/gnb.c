#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Paging.h" // Include the Paging structure definition
#include "per_encoder.h" // Include functions for PER encoding
#include "asn_application.h" // Include for ASN.1 infrastructure functions

#define PORT 8080

volatile int sfn = 0; // System Frame Number

void* update_sfn() {
    while (1) {
        sleep(1); // Wait for 1 second
        sfn = (sfn + 1) % 1024; // Increment SFN and reset after reaching 1023
        printf("[SFN: %d]\n", sfn);
    }
}

void init_and_send_paging_message(uint8_t* buffer, size_t buffer_size) {
    Paging_t *pagingMessage = calloc(1, sizeof(Paging_t)); // Allocate and zero out a new Paging message
    if (!pagingMessage) {
        perror("calloc failed for Paging");
        exit(1);
    }

    // Allocate memory for the PagingRecord
    struct PagingRecord *pagingRecord = calloc(1, sizeof(struct PagingRecord));
    if (!pagingRecord) {
        perror("calloc failed for PagingRecord");
        exit(1);
    }

    // Initialize the ue_Identity field
    pagingRecord->ue_Identity.present = PagingUE_Identity_PR_ng_5G_S_TMSI; // Assuming you want to use ng_5G_S_TMSI
    pagingRecord->ue_Identity.choice.ng_5G_S_TMSI.buf = calloc(1, sizeof(uint32_t)); // Assuming ng_5G_S_TMSI is 4 bytes
    if (!pagingRecord->ue_Identity.choice.ng_5G_S_TMSI.buf) {
        perror("calloc failed for ng_5G_S_TMSI buffer");
        exit(1);
    }
    uint32_t ng_5G_S_TMSI_value = 12345; // Example ng_5G_S_TMSI value
    memcpy(pagingRecord->ue_Identity.choice.ng_5G_S_TMSI.buf, &ng_5G_S_TMSI_value, sizeof(uint32_t));
    pagingRecord->ue_Identity.choice.ng_5G_S_TMSI.size = sizeof(uint32_t); // Size of ng_5G_S_TMSI

    // Add the PagingRecord to the Paging message
    if (!pagingMessage->pagingRecordList) {
        pagingMessage->pagingRecordList = calloc(1, sizeof(struct PagingRecordList));
        if (!pagingMessage->pagingRecordList) {
            perror("calloc failed for PagingRecordList");
            exit(1);
        }
    }
    //ASN_SEQUENCE_ADD(&pagingMessage->pagingRecordList->list, pagingRecord);

    // Encode the Paging message using PER
    uint8_t buffers[1024]; // Buffer to store the encoded message
    memset(buffers, 0, 1024); // Clear the buffer

    asn_enc_rval_t ec; // Encoder return value
    ec = uper_encode_to_buffer(&asn_DEF_Paging, pagingMessage, buffers, 1024);
    if(ec.encoded == -1) {
        fprintf(stderr, "Could not encode Paging (at %s)\n", ec.failed_type ? ec.failed_type->name : "unknown");
        exit(1);
    }

    // Send the encoded Paging message to the UE
    // Assuming you have a function send_to_ue that takes the buffer and its size
    // send_to_ue(buffer, (ec.encoded + 7) / 8); // +7 to handle partial bytes


    // Free the allocated Paging message
    ASN_STRUCT_FREE(asn_DEF_Paging, pagingMessage);
}

void* udp_server() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    unsigned int len = sizeof(cliaddr); //len is value/resuslt

    while (1) {
        char buffer[1024];
        size_t buffer_size = sizeof(buffer);
        int n = recvfrom(sockfd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Client: %s\n", buffer);

        init_and_send_paging_message(buffer, buffer_size);
        sendto(sockfd, buffer, buffer_size, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
        printf("Message sent.\n");
    }
}

int main() {
    pthread_t thread1, thread2;

    // Creating two threads, one for updating SFN and another for UDP server
    if (pthread_create(&thread1, NULL, update_sfn, NULL) != 0) {
        perror("Failed to create thread for updating SFN");
        return 1;
    }

    if (pthread_create(&thread2, NULL, udp_server, NULL) != 0) {
        perror("Failed to create thread for UDP server");
        return 1;
    }

    // Joining threads with the main thread
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}