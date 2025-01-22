#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

// #define SERVER_PORT 12345
// #define SERVER_IP "127.0.0.1"
// #define MAX_BUFFER_SIZE 1024 
// #define NUM_SAMPLES 1000000
// #define SIGMA 0.025 
// #define LIMIT 3.0   

// void send_array(double *array, size_t size, const char *host)
// {
//     int sock;
//     struct sockaddr_in server_addr;
//     sock = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sock < 0) 
//     {
//         perror("Ошибка создания сокета");
//         exit(EXIT_FAILURE);
//     }
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(SERVER_PORT);
//     inet_pton(AF_INET, host, &server_addr.sin_addr);
//     for (size_t i = 0; i < size; i++)
//     {
//         char buffer[MAX_BUFFER_SIZE];
//         snprintf(buffer, sizeof(buffer), "%f", array[i]);
//         ssize_t sent_bytes = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
//         if (sent_bytes < 0) 
//         {
//             perror("Ошибка отправки данных");
//         }
//         usleep(1000);
//     }
//     close(sock);
// }

float model_fly()
{
    float h=1000;
    return h;
}

int main(void)
{
    
    
    return 0;
}