#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#define PORT 8080
#define BUF_SIZE 1024
#define PACKET_SIZE 40

// Инвертирование знака 16-битного числа как в Python
int16_t invert(uint16_t ch)
{
    if (ch & 0x8000)
    {
        ch = (~ch) & 0xFFFF;
        ch += 1;
        return -((int16_t)ch);
    }
    return (int16_t)ch;
}

// Распаковка и вывод содержимого пакета
void process_packet(uint8_t *data)
{
    // Склейка времени из 8 байт
    uint64_t t = 0;
    for (int i = 0; i < 8; i++)
    {
        t = (t << 8) | data[i];
    }

    int index = 8;
    int16_t values[11];
    for (int i = 0; i < 12; ++i)
    {
        uint16_t raw = (data[index] << 8) | data[index + 1];
        values[i] = invert(raw);
        index += 2;
    }

    printf(" t = %lu | lenght= %d | baro= %f | gyro = [%f %f %f] | accl = [%f %f %f] | magn = [%f %f %f]\n",
           t, values[0], values[1] * 0.04, values[2] * 0.02, values[3] * 0.02, values[4] * 0.02,
           values[5] * 0.0008, values[6] * 0.0008, values[7] * 0.0008,
           values[8] * 0.1, values[9] * 0.1, values[10] * 0.1);
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    uint8_t buffer[BUF_SIZE];

    // Создание UDP-сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Привязка адреса
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // или INADDR_ANY
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Установка сокета в неблокирующий режим
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    printf("Listening for UDP packets on port %d...\n", PORT);

    while (1)
    {
        ssize_t n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (n == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("recvfrom error");
            }
            usleep(1000); // уменьшить CPU usage
            continue;
        }

        if (n == PACKET_SIZE)
        {
            process_packet(buffer);
        }
        else
        {
            printf("Invalid packet size: %ld bytes\n", n);
        }
    }

    close(sockfd);
    return 0;
}