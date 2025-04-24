#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sqlite3.h>
#include "Barometr.h"
#include "gyroscopes.h"
#include "accelerometrs.h"
#include "Magnetometer.h"
#include "model_flight.h"

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER_SIZE 1024

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

// отправка случайных значений для отрисовки графика по udp
void send_data(double h_m, double ox_c, double oy_c, double oz_c, double vx_msec, double vy_msec, double vz_msec, double vox_csec, double voy_csec, double voz_csec, double ax_m2sec, double ay_m2sec, double az_m2sec, double x_mG, double y_mG, double z_mG, size_t size, const char *host)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    DataPacket packet;
    packet.Time_nsec = ts.tv_nsec;
    packet.h_mbar = (int)(h_m / 0.64);
    packet.ox_c = (int)(ox_c / 0.32);
    packet.oy_c = (int)(oy_c / 0.32);
    packet.oz_c = (int)(oz_c / 0.32);
    packet.vx_msec = (int)(vx_msec / 0.008);
    packet.vy_msec = (int)(vy_msec / 0.008);
    packet.vz_msec = (int)(vz_msec / 0.008);
    packet.vox_csec = (int)(vox_csec / 0.32);
    packet.voy_csec = (int)(voy_csec / 0.32);
    packet.voz_csec = (int)(voz_csec / 0.32);
    packet.ax_m2sec = (int)(ax_m2sec / 0.008);
    packet.ay_m2sec = (int)(ay_m2sec / 0.008);
    packet.az_m2sec = (int)(az_m2sec / 0.008);
    packet.x_mG = (int)(x_mG / 0.1);
    packet.y_mG = (int)(y_mG / 0.1);
    packet.z_mG = (int)(z_mG / 0.1);
    packet.length = sizeof(DataPacket);
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    ssize_t sent = sendto(sock, &packet, packet.length, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (sent < 0)
    {
        perror("senddto failed");
    }
    close(sock);
}

// получение из бд
int data_bd(int time_request, int *time, double *X, double *Y, double *h_m, char **direction, double *ox_c, double *oy_c, double *oz_c, double *vx_msec, double *vy_msec, double *vz_msec, double *vox_csec, double *voy_csec, double *voz_csec, double *ax_m2sec, double *ay_m2sec, double *az_m2sec, double *Bx_G, double *By_G, double *Bz_G) // получение данных из бд
{
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char *sql = "SELECT * FROM model_flight WHERE Time_sec=?";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(res, 1, time_request);
        while (sqlite3_step(res) == SQLITE_ROW)
        {
            *time = sqlite3_column_int(res, 0);
            *X = sqlite3_column_double(res, 1);
            *Y = sqlite3_column_double(res, 2);
            *h_m = sqlite3_column_double(res, 3);
            strcpy(*direction, sqlite3_column_text(res, 4));
            *ox_c = sqlite3_column_double(res, 5);
            *oy_c = sqlite3_column_double(res, 6);
            *oz_c = sqlite3_column_double(res, 7);
            *vx_msec = sqlite3_column_double(res, 8);
            *vy_msec = sqlite3_column_double(res, 9);
            *vz_msec = sqlite3_column_double(res, 10);
            *vox_csec = sqlite3_column_double(res, 11);
            *voy_csec = sqlite3_column_double(res, 12);
            *voz_csec = sqlite3_column_double(res, 13);
            *ax_m2sec = sqlite3_column_double(res, 14);
            *ay_m2sec = sqlite3_column_double(res, 15);
            *az_m2sec = sqlite3_column_double(res, 16);
            *Bx_G = sqlite3_column_double(res, 17);
            *By_G = sqlite3_column_double(res, 18);
            *Bz_G = sqlite3_column_double(res, 19);
        }
    }
    else
    {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}
// запись в бд
int write_bd(int time, double X, double Y, double h_mbar, char **direction, double roll_grad, double pitch_grad, double yaw_grad, double X_axis_m_sec, double Y_axis_m_sec, double Z_axis_m_sec, double vox_csec, double voy_csec, double voz_csec, double ax_m2sec, double ay_m2sec, double az_m2sec, double x_mG, double y_mG, double z_mG, double declination_c, double inclination_c)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    const char *sql = "INSERT INTO finish_data VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22)";
    char *err_msg = 0;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(res, 1, time);
        sqlite3_bind_double(res, 2, X);
        sqlite3_bind_double(res, 3, Y);
        sqlite3_bind_double(res, 4, h_mbar);
        sqlite3_bind_text(res, 5, *direction, -1, SQLITE_STATIC);
        sqlite3_bind_double(res, 6, roll_grad);
        sqlite3_bind_double(res, 7, pitch_grad);
        sqlite3_bind_double(res, 8, yaw_grad);
        sqlite3_bind_double(res, 9, X_axis_m_sec);
        sqlite3_bind_double(res, 10, Y_axis_m_sec);
        sqlite3_bind_double(res, 11, Z_axis_m_sec);
        sqlite3_bind_double(res, 12, vox_csec);
        sqlite3_bind_double(res, 13, voy_csec);
        sqlite3_bind_double(res, 14, voz_csec);
        sqlite3_bind_double(res, 15, ax_m2sec);
        sqlite3_bind_double(res, 16, ay_m2sec);
        sqlite3_bind_double(res, 17, az_m2sec);
        sqlite3_bind_double(res, 18, x_mG);
        sqlite3_bind_double(res, 19, y_mG);
        sqlite3_bind_double(res, 20, z_mG);
        sqlite3_bind_double(res, 21, declination_c);
        sqlite3_bind_double(res, 22, inclination_c);
        while (rc = sqlite3_step(res) == SQLITE_ROW)
        {
        }
    }
    else
    {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}

int main(void)
{
    int num_model;
    while (true)
    {
        printf("Введите модель полета(1-Висение;2-Линейный полет;3-взлет->движение->посадка;4-движение с поворотом;5-равномерное движение с маневром): ");
        // выбор модели полета
        scanf("%d", &num_model);
        if (num_model == 1 || num_model == 2 || num_model == 3 || num_model == 4 || num_model == 5)
        {
            break;
        }
        else
        {
            printf("Такой модели полета нет. Введите еще раз\n");
        }
    }
    // вызов модели полета
    flight(num_model);
    // время запроса
    int time_request = 0;
    // время которое возвращается
    int time_sec;
    double X, Y, h_m, ox_c, oy_c, oz_c, vx_msec, vy_msec, vz_msec, vox_csec, voy_csec, voz_csec, ax_m2sec, ay_m2sec, az_m2sec, Bx_G, By_G, Bz_G, declination_c = 0, inclination_c = 0;
    char *direction = malloc(5);
    double sys_er;
    // время работы
    int work_time = get_time();
    while (true)
    {
        printf("Введите погрешность от -4.5 до 4.5: ");
        // ввод системной ошибки
        scanf("%lf", &sys_er);
        if (sys_er >= -4.5 && sys_er <= 4.5)
        {
            break;
        }
        else
        {
            printf("Неверная системная ошибка. Введите еще раз\n");
        }
    }
    // int NUM_SAMPLES = 2;
    // size_t size = NUM_SAMPLES;
    printf("%-10s| %-10s|%-42s | %-43s | %-40s | %s\n", "Время(сек):", "Курс:", "Гироскоп(град):", "Акселерометр(g):", "Магнетометер(mG):", "Барометр(mbar):");
    for (int i = 0; i < work_time; i++)
    {
        data_bd(time_request, &time_sec, &X, &Y, &h_m, &direction, &ox_c, &oy_c, &oz_c, &vx_msec, &vy_msec, &vz_msec, &vox_csec, &voy_csec, &voz_csec, &ax_m2sec, &ay_m2sec, &az_m2sec, &Bx_G, &By_G, &Bz_G);
        double data_roll_grad, data_pitch_grad, data_yaw_grad;
        double Y_axis_m_sec, Z_axis_m_sec, X_axis_m_sec;
        double data_x_mG, data_y_mG, data_z_mG;
        // данные с гироскопа
        data_gyro(&vox_csec, &voy_csec, &voz_csec, ox_c, oy_c, oz_c, num_model, time_request, work_time, &data_roll_grad, &data_pitch_grad, &data_yaw_grad);
        // данные с акселерометра
        data_accel(&ax_m2sec, &ay_m2sec, &az_m2sec, vx_msec, vy_msec, vz_msec, num_model, time_request, work_time, &Y_axis_m_sec, &X_axis_m_sec, &Z_axis_m_sec);
        // данные с магнетометра
        data_mag(Bx_G, By_G, Bz_G, data_roll_grad, data_pitch_grad, data_yaw_grad, time_request, work_time, &data_x_mG, &data_y_mG, &data_z_mG, &declination_c, &inclination_c);
        // данные с барометра
        double bar_mbar = data_bar(h_m, sys_er, time_request, work_time);
        printf("%-10d | %-10s | (%f;%f;%f) | (%f;%f;%f) | (%f;%f;%f) | %f\n", i, direction, data_roll_grad, data_pitch_grad, data_yaw_grad, X_axis_m_sec, Y_axis_m_sec, Z_axis_m_sec, data_x_mG, data_y_mG, data_z_mG, bar_mbar);
        time_request++;
        write_bd(i, X, Y, bar_mbar, &direction, data_roll_grad, data_pitch_grad, data_yaw_grad, X_axis_m_sec, Y_axis_m_sec, Z_axis_m_sec, vox_csec, voy_csec, voz_csec, ax_m2sec, ay_m2sec, az_m2sec, data_x_mG, data_y_mG, data_z_mG, declination_c, inclination_c);
        send_data(bar_mbar, data_roll_grad, data_pitch_grad, data_yaw_grad, X_axis_m_sec, Y_axis_m_sec, Z_axis_m_sec, vox_csec, voy_csec, voz_csec, ax_m2sec, ay_m2sec, az_m2sec, data_x_mG, data_y_mG, data_z_mG, 2, SERVER_IP);
    }
    free(direction);
    return 0;
}