#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_LINE_SIZE 100
#define PORT 8080

int num_records;

typedef struct {
    int player_id;
    int arrival_time;
    char gender;
    char preference;
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

        record.player_id = atoi(token);

        // Parse req_type
        token = strtok(NULL, ",");

        record.arrival_time=atoi(token);

        // Parse room_no
        token = strtok(NULL, ",");
        record.gender = token[0];

        // Parse slot_start_time
        token = strtok(NULL, ",");
        record.preference=token[0];

        // Store the record
        records[i] = record;
        i++;
    }

    fclose(file);

    // Return the number of records
    *num_records = count;

    return records;
}



void sendRequest(int player_id, int arrival_time, char gender, char preference) {
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
    snprintf(buffer, sizeof(buffer), "%d %d %c %c", player_id, arrival_time, gender, preference);

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
        sendRequest(records[i].player_id, records[i].arrival_time, records[i].gender, records[i].preference);
       // sleep(0.001);
    }

    // Free allocated memory
    free(records);

    return 0;
}
