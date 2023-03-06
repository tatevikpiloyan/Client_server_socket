#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 3001
#define MAX_CLIENT_NUMBER 5
#define BUFFER_SIZE 1024

int spliter(char* str, const char delimiter, char*** arr) {

    int count = 1;
    char* tmp = str;

    while (*tmp == delimiter)
    {
        ++tmp;
    }

    char* new_tmp = tmp;
    while (*(new_tmp++))
    {
        if (*new_tmp == delimiter && *(new_tmp + 1) != delimiter && *(new_tmp + 1) != 0)
        {
            ++count;
        }
    }

    (*arr) = malloc(sizeof(char*) * count);
    char* ts = NULL;

    for(int i = 0; i < count; i++)
    {
        ts = tmp;
        while (*tmp != delimiter && *tmp != 0)
        {
            ++tmp;
        }
        int length = (tmp - ts + 1);
        (*arr)[i] = malloc(sizeof(char) * length);
        memcpy((*arr)[i], ts, sizeof(char) * (length - 1));
        (*arr)[i][length - 1] = 0;

        while (*tmp == delimiter)
        {
            ++tmp;
        }
    }
    return count;
}

char* command_handler(const char* buffer)
{
    FILE* fs;

    char* output = NULL;
    char tmp[BUFFER_SIZE];
    size_t output_size = 0;

    const char failure[] = "Error: Command failure\n";
    
    fs = popen(buffer, "r");
    if (!fs)
    {
        output = realloc(output, strlen(failure) + 1);
        strcpy(output, failure);
        return output;
    }

    while (fgets(tmp, BUFFER_SIZE, fs))
    {
        output_size += strlen(tmp);
        output = realloc(output, output_size);
        if (!output)
        {
            strcpy(output, "Error: Out of memory\n");
            break;
        }
        strcat(output, tmp);
    }

    if (!output)
    {
        output = realloc(output, 1);
        strcpy(output, "\n");
    }

    if (pclose(fs))
    {
        output = realloc(output, strlen(failure) + 1);
        strcpy(output, failure);
    }
    return output;
}

void* chat_handler(void* arg)
{
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    char* answer = NULL;

    char** parsed = NULL;
    char delimiter;
    int count = 0;
    char tmp[BUFFER_SIZE];

    while(1)
    {
        int recieve_size;
        recieve_size = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (recieve_size < 1 || !strcmp(buffer, "disconnect"))
        {
            printf("Client> Connection Closed\n");
            close(client_socket);
            pthread_exit(NULL);
        }
        printf("Client> %s\n", buffer);

        strcpy(tmp, buffer);
        delimiter = '\"';
        count = spliter(tmp, delimiter, &parsed);
        if (count != 2)
        {
            memset(buffer,'\0',strlen(buffer));
        }
        else
        {
            strcpy(buffer, parsed[1]);
        }
        strcpy(tmp, parsed[0]);
        delimiter = ' ';
        
        for (int i = 0; i < count; ++i) 
        {
            free(parsed[i]);
        }
        free(parsed);

        if ((count = spliter(tmp, delimiter, &parsed)) > 1 || strcmp(parsed[0], "shell"))
        {
            memset(buffer,'\0',strlen(buffer));
        }

        for (int i = 0; i < count; ++i) 
        {
            free (parsed[i]);
        }
        free(parsed);

        answer = command_handler(buffer);
        if (!answer)
        {
            answer = "\n";
        }

        printf("Server> %s", answer);

        int send_size;
        send_size = send(client_socket, answer, BUFFER_SIZE, 0);

        if (send_size < 0)
        {
            free(answer);
            perror("Failure: Error sending message");
            exit(EXIT_FAILURE);
        }
        free(answer);
        answer = NULL;
    }
    pthread_exit(NULL);
}

void connection_handler(int server_socket)
{
    int client_socket;

    pthread_t thread_id;

    while (1)
    {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0)
        {
            perror("Failure: Error connection with client");
            exit(EXIT_FAILURE);
        }
        printf("Success: Client connected\n");

        if (pthread_create(&thread_id, NULL, chat_handler, &client_socket))
        {
            perror("Failure: Couldn't create thread");
            exit(EXIT_FAILURE);
        }
        sleep(5);
    }
    close(server_socket);
}

int main()
{
    int server_socket;
    struct sockaddr_in server_address;

    int option_value = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Failure: Error with server socket");
        exit(EXIT_FAILURE);
    }
    printf("Success: Server socket created\n");

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)))
    {
        perror("Failure: Error with socket options");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)))
    {
        perror("Failure: Error binding address with socket");
        exit(EXIT_FAILURE);
    }
    printf("Success: Binding address with socket\n");

    if (listen(server_socket, MAX_CLIENT_NUMBER) < 0)
    {
        perror("Failure: Error litening to connections");
        exit(EXIT_FAILURE);
    }
    printf("Success: Listening...\n");

    connection_handler(server_socket);
    close(server_socket);
}