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
    // 2 байт - 16 бит
    int length;
    // 8 байт - 64 бита
    uint64_t Time_nsec;
    // 2 байта - 16 бита
    int h_mbar;
    int ox_c;
    int oy_c;
    int oz_c;
    int vx_msec;
    int vy_msec;
    int vz_msec;
    int vox_csec;
    int voy_csec;
    int voz_csec;
    int ax_m2sec;
    int ay_m2sec;
    int az_m2sec;
    int x_mG;
    int y_mG;
    int z_mG;
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
            printf("  H_mbar: %f\n", packet.h_mbar * 0.64);
            printf("  Ox_c: %f\n", packet.ox_c * 0.32);
            printf("  Oy_c: %f\n", packet.oy_c * 0.32);
            printf("  Oz_c: %f\n", packet.oz_c * 0.32);
            printf("  Vx_msec: %f\n", packet.vx_msec * 0.008);
            printf("  Vy_msec: %f\n", packet.vy_msec * 0.008);
            printf("  Vz_msec: %f\n", packet.vz_msec * 0.008);
            printf("  Vox_csec: %f\n", packet.vox_csec * 0.32);
            printf("  Voy_csec: %f\n", packet.voy_csec * 0.32);
            printf("  Voz_csec: %f\n", packet.voz_csec * 0.32);
            printf("  Ax_m2sec: %f\n", packet.ax_m2sec * 0.008);
            printf("  Ay_m2sec: %f\n", packet.ay_m2sec * 0.008);
            printf("  Az_m2sec: %f\n", packet.az_m2sec * 0.008);
            printf("  X_mG: %f\n", packet.x_mG * 0.1);
            printf("  Y_mG: %f\n", packet.y_mG * 0.1);
            printf("  Z_mG: %f\n", packet.z_mG * 0.1);
        }
        else
        {
            printf("Получен неполный пакет (%ld байт,%ld)\n", received, sizeof(packet));
        }
    }

    close(sock);
    return 0;
}
