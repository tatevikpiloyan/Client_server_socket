#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>

#define PORT 3001
#define BUFFER_SIZE 1024

void chat_handler(int client_socket)
{
    char buffer[BUFFER_SIZE];

    int send_size = 0;
    int recieve_size = 0;

    while (1)
    {
        printf("Client> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer) - 1] = '\0';

        send_size = send(client_socket, buffer, BUFFER_SIZE, 0);
        if (send_size < 0)
        {
            perror("Failure: Error sending message");
            exit(EXIT_FAILURE);
        }

        if (!strcmp(buffer, "disconnect"))
        {
            close(client_socket);
            exit(EXIT_SUCCESS);
        }

        recieve_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recieve_size < 0)
        {
            perror("Failure: Error receiving message");
            exit(EXIT_FAILURE);
        }
        printf("Server> %s", buffer);
    }
    close(client_socket);
}

int main(int argc, char* argv[])
{
    char address[] = "127.0.0.1";
    if (argc < 4 || strcmp(argv[2], address) || (atoi(argv[3]) != PORT))
    {
        printf("Error: Too few arguments\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "connect"))
    {
        printf("Error: Wrong request\n");
        exit(EXIT_FAILURE);
    }

    int client_socket;

    struct sockaddr_in client_address;
    struct sockaddr_in server_address;

    char buffer[BUFFER_SIZE];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Failure: Error with client socket");
        exit(EXIT_FAILURE);
    }
    printf("Success: Client socket created\n"); 

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)))
    {
        perror("Failure: Error to connect server");
        exit(EXIT_FAILURE);
    }
    printf("Success: Connected to server\n");
    chat_handler(client_socket);
}