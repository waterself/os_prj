#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 4000
#define MAXUSER 4
#define BUFSIZE 224
#define USERNAMESIZE 32

// global variable, when access, mutex lock require
int client_count = 0;
int client_fd_arr[MAXUSER];
pthread_t tid[MAXUSER];
pthread_mutex_t mutx;

void *client_control_thread(void *arg)
{
    int client_index = *(int *)arg;
    printf("connected: ");
    printf("%d\n", client_index);
    int has_name = 0;
    int str_len = 0;
    char username[USERNAMESIZE];
    char buf[BUFSIZE];
    char message[USERNAMESIZE + BUFSIZE];


    if (has_name == 0) //get name and notify
    {
        write(client_fd_arr[client_index], "enter your name:\0", 17);
        str_len = read(client_fd_arr[client_index], username, USERNAMESIZE + BUFSIZE);
        has_name = str_len;

        memset(message, 0, sizeof(message));
        sprintf(message, "welcome, %s!\0", username);
        write(client_fd_arr[client_index], message, USERNAMESIZE + BUFSIZE);

        memset(message, 0, sizeof(message));
        sprintf(message, "%s entered\0", username);
        broadcast_message(message, client_index);

        memset(message, 0, sizeof(message));
        sprintf(message, "current users: %d\0", client_count + 1);
        broadcast_message(message, client_index);
        write(client_fd_arr[client_index], message, USERNAMESIZE + BUFSIZE);

        memset(message, 0, sizeof(message));
        memset(buf, 0, sizeof(buf));
        str_len = 0;
    }

    do //read from client
    {
        memset(message, 0, sizeof(message));
        memset(buf, 0, sizeof(buf));
        str_len = read(client_fd_arr[client_index], buf, BUFSIZE - 1);

        if (strcmp(buf, "quit") == 0) // check quit
        {
            break;
        }

        sprintf(message, "%s: %s", username, buf);
        broadcast_message(message, client_index);

    } while (str_len > -1); // connect end
    //notify exit
    memset(message, 0, sizeof(message));
    sprintf(message, "%s has exit\ncurrent user: %d\n", username, client_count);
    printf(message);
    broadcast_message(message, client_index);
    pthread_mutex_lock(&mutx);
    // critical section enter
    for (int i = 0; i <= client_count; i++)
    {
        if (client_index == client_fd_arr[i])
        {
            //push for fill low index arr
            while (i++ < client_count - 1)
                client_fd_arr[i] = client_fd_arr[i + 1]; 
            break;
        }
    }
    client_count--;
    close(client_fd_arr[client_index]);
    client_fd_arr[client_index] = 0;
    pthread_mutex_unlock(&mutx);
    // critical section end
    return NULL;
}

void broadcast_message(char *message, int self)
{
    // critical section start
    for (int i = 0; i <= client_count; i++)
    {
        if (i != self)
        {
            write(client_fd_arr[i], message, USERNAMESIZE + BUFSIZE);
        }
    }
    // critical section end
    pthread_mutex_unlock(&mutx);
}

int main()
{
    int server_fd;
    int client_fd;
    int client_addr_size;
    int user_count = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    printf("declare done\n");

    // IPv4, TCP/IP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("server socket error\n");
        return -1;
    }
    printf("server socket done\n");

    // set memory for server'
    memset(&server_addr, 0, sizeof(server_addr));
    printf("set Memory done\n");

    // server socket struct set
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    printf("set server_socket struct set done\n");

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("server bind error\n");
        return -1;
    }

    printf("set bind done\n");
    if (listen(server_fd, 10) < 0)
    {

        printf("server listen error\n");
        return -1;
    }

    printf("set listen done\n");

    printf("server on\n");
    while (1)
    {
        // check for connecting
        client_addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        
        if (client_fd < 0)
        {
            printf("accept fail\n");
            return -1;
        }
        else
        {
            pthread_mutex_lock(&mutx);
            // critical section enter
            if (client_fd_arr[client_count] == 0)
            {
                client_fd_arr[client_count] = client_fd;
                pthread_create(&tid[client_count], NULL, &client_control_thread, &client_count);
                pthread_detach(client_fd);
            }
            else
            {
                client_count++;
                client_fd_arr[client_count] = client_fd;
                pthread_create(&tid[client_count], NULL, &client_control_thread, &client_count);
                pthread_detach(client_fd);
            }
            // critical section end
            pthread_mutex_unlock(&mutx);
        }
    }
    close(server_fd);
    return 0;
}
