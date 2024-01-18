#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_LINE_SIZE 100
#define PORT 8080

int num_records;

typedef struct {
    char req_timestamp[50];
    char req_type[50];
    int room_no;
    char slot_start_time[50];
} Record;


Record* read_csv(const char *filename, int *num_records) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return NULL;
    }

    // Count the number of lines in the file
    int count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    count--;

    // Allocate memory for the records
    Record *records = malloc(sizeof(Record) * count);
    if (records == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    // Go back to the start of the file
    rewind(file);

    char line[MAX_LINE_SIZE];

    if (fgets(line, sizeof(line), file) == NULL) {
        // Handle error or empty file
        fclose(file);
        free(records);
        return NULL;
    }

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        Record record;
        char *token;

        // Parse req_timestamp
        token = strtok(line, ",");

        if (token == NULL) {
            printf("Error parsing req_timestamp\n");
            fclose(file);
            free(records);
            return NULL;
        }


        strncpy(record.req_timestamp, token, sizeof(record.req_timestamp) - 1);
        record.req_timestamp[sizeof(record.req_timestamp) - 1] = '\0';

        // Parse req_type
        token = strtok(NULL, ",");
        if (token == NULL) {
            printf("Error parsing req_type\n");
            fclose(file);
            free(records);
            return NULL;
        }

        strncpy(record.req_type, token, sizeof(record.req_type) - 1);
        record.req_type[sizeof(record.req_type) - 1] = '\0';

         if (strcmp(token, "GET") == 0) {
            // Store the record
            records[i] = record;
            i++;
            continue;
         }
        // Parse room_no
        token = strtok(NULL, ",");
        record.room_no = atoi(token);

        // Parse slot_start_time
        token = strtok(NULL, ",");

        strncpy(record.slot_start_time, token, sizeof(record.slot_start_time) - 1);

        token[strcspn(token, "\n")] = '\0';
        record.slot_start_time[sizeof(record.slot_start_time) - 1] = '\0';

        // Store the record
        records[i] = record;
        i++;
    }

    fclose(file);

    // Return the number of records
    *num_records = count;

    return records;
}



void sendRequest(const char *req_timestamp, const char *req_type, int room_no, const char *slot_start_time) {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        printf("Error creating socket.\n");
        exit(1);
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        printf("Error connecting to server.\n");
        close(clientSocket);
        exit(1);
    }

    // Prepare and send request
    char buffer[1024];
    if(strcmp(req_type,"GET")==0){
        snprintf(buffer, sizeof(buffer), "%s %s", req_timestamp, req_type);
    } else{
        snprintf(buffer, sizeof(buffer), "%s %s %d %s", req_timestamp, req_type, room_no, slot_start_time);
    }

    int sendStatus = send(clientSocket, buffer, strlen(buffer) + 1, 0);  // Use strlen(buffer) + 1 instead of sizeof(buffer)
    if (sendStatus == -1) {
        perror("Error sending data");
    // Handle the error or exit the client
    }
    // Close the socket
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if(argc<2||argc>2){
        printf("Error! Execute as $ ./client <input file name>");
    }
    char *filename = argv[1];
    Record *records = read_csv(filename, &num_records);

    if (records == NULL) {
        // Handle error
        return 1;
    }

    for (int i = 0; i < num_records; i++) {
        // Assuming each record in the CSV file corresponds to a request
        sendRequest(records[i].req_timestamp, records[i].req_type, records[i].room_no, records[i].slot_start_time);
        sleep(0.001);
    }

    // Free allocated memory
    free(records);

    return 0;
}
