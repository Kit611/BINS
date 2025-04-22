#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define BUF_SIZE 1024

#pragma pack(push, 1)
typedef struct
{
    uint16_t length;    // 2 байт - 16 бит
    uint64_t Time_nsec; // 8 байт - 64 бита
    float h_mbar;       // 4 байта - 32 бита
    float ox_c;
    float oy_c;
    float oz_c;
    float vx_msec;
    float vy_msec;
    float vz_msec;
    float vox_csec;
    float voy_csec;
    float voz_csec;
    float ax_m2sec;
    float ay_m2sec;
    float az_m2sec;
    uint16_t x_mG;
    uint16_t y_mG;
    uint16_t z_mG;
} DataPacket;
#pragma pack(pop)

int main()
{
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Создаем UDP сокет
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
        return 1;
    }

    // Заполняем адрес и порт, на котором будем слушать
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); // тот же порт, что и у отправителя
    server_addr.sin_addr.s_addr = INADDR_ANY;  // принимать со всех интерфейсов

    // Привязываем сокет
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        return 1;
    }

    printf("UDP приемник запущен. Ожидание данных...\n");

    while (1)
    {
        DataPacket packet;
        ssize_t received = recvfrom(sock, &packet, sizeof(packet), 0,
                                    (struct sockaddr *)&client_addr, &addr_len);

        if (received < 0)
        {
            perror("recvfrom failed");
        }
        else if (received == sizeof(packet))
        {
            printf("Принят пакет:\n");
            printf("  Len: %d\n", packet.length);
            printf("  Time: %ld\n", packet.Time_nsec);
            printf("  H_mbar: %.2f\n", packet.h_mbar);
            printf("  Ox_c: %.2f\n", packet.ox_c);
            printf("  Oy_c: %.2f\n", packet.oy_c);
            printf("  Oz_c: %.2f\n", packet.oz_c);
            printf("  Vx_msec: %.2f\n", packet.vx_msec);
            printf("  Vy_msec: %.2f\n", packet.vy_msec);
            printf("  Vz_msec: %.2f\n", packet.vz_msec);
            printf("  Vox_csec: %.2f\n", packet.vox_csec);
            printf("  Voy_csec: %.2f\n", packet.voy_csec);
            printf("  Voz_csec: %.2f\n", packet.voz_csec);
            printf("  Ax_m2sec: %.2f\n", packet.ax_m2sec);
            printf("  Ay_m2sec: %.2f\n", packet.ay_m2sec);
            printf("  Az_m2sec: %.2f\n", packet.az_m2sec);
            printf("  X_mG: %d\n", packet.x_mG);
            printf("  Y_mG: %d\n", packet.y_mG);
            printf("  Z_mG: %d\n", packet.z_mG);
        }
        else
        {
            printf("Получен неполный пакет (%ld байт,%ld)\n", received, sizeof(packet));
        }
    }

    close(sock);
    return 0;
}
