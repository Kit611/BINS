#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_ACCEL (1.29) // значение сигма для генерации
#define LIMIT (3.0)        // ограничения дипозона

// генерация случайных значений нормальным распределением
double generate_normal_accel()
{
    static int has_spare = 0;
    static double spare;

    if (has_spare)
    {
        has_spare = 0;
        return spare;
    }

    double u1, u2, s, z0, z1;

    do
    {
        u1 = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        u2 = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        s = u1 * u1 + u2 * u2;
    } while (s >= 1 || s == 0);

    double factor = sqrt(-2.0 * log(s) / s);
    z0 = u1 * factor * SIGMA_BAR;
    z1 = u2 * factor * SIGMA_BAR;

    if (z0 < -LIMIT * SIGMA_BAR)
        z0 = -LIMIT * SIGMA_BAR;
    if (z0 > LIMIT * SIGMA_BAR)
        z0 = LIMIT * SIGMA_BAR;
    if (z1 < -LIMIT * SIGMA_BAR)
        z1 = -LIMIT * SIGMA_BAR;
    if (z1 > LIMIT * SIGMA_BAR)
        z1 = LIMIT * SIGMA_BAR;

    spare = z1;
    has_spare = 1;
    return z0;
}

// функция интегрирования для получения скорости
double integrate(double acceleration, double t_start, double t_end)
{
    double dt = t_end - t_start;
    double initial_velocity = 0;
    double final_velocity = initial_velocity + acceleration * dt;
    return final_velocity;
}

double vx, vy, vz;

double data_accel(double *aX_m2sec, double *aY_m2sec, double *aZ_m2sec, double vx_msec, double vy_msec, double vz_msec, int num, double time_request, int count, double *Y_axis_msec, double *X_axis_msec, double *Z_axis_msec) // главная функция
{
    double X_msec, Y_msec, Z_msec;
    if (num == 3 || num == 4 || num == 5)
    {
        if (*aX_m2sec != 0 || *aY_m2sec != 0 || *aZ_m2sec != 1 || *aZ_m2sec != -1)
        {
            double t_start_x = time_request - 1, t_end_x = time_request;
            double t_start_y = time_request - 1, t_end_y = time_request;
            double t_start_z = time_request - 1, t_end_z = time_request;
            X_msec = integrate(*aX_m2sec, t_start_x, t_end_x);
            Y_msec = integrate(*aY_m2sec, t_start_y, t_end_y); // преобразование в скорость
            Z_msec = integrate(*aZ_m2sec, t_start_z, t_end_z);
            vy += Y_msec;
            if (X_msec < 64)
            {
                vx += X_msec;
            }
            else
            {
                vx = 13;
            }
            if (Z_msec < 6 && Z_msec > 0)
            {
                vz += Z_msec - 1;
            }
            else if (time_request < 53)
            {
                vz = 6;
            }
            else
            {
                vz = 0;
            }
        }
    }
    else if (num == 2)
    {
        X_msec = vx_msec;
        Y_msec = vy_msec;
        Z_msec = vz_msec;
    }
    X_msec += generate_normal_accel();
    Y_msec += generate_normal_accel();
    Z_msec += generate_normal_accel();
    *X_axis_msec = X_msec;
    *Y_axis_msec = Y_msec;
    *Z_axis_msec = Z_msec;
    *aX_m2sec += generate_normal_accel();
    *aY_m2sec += generate_normal_accel();
    *aZ_m2sec += generate_normal_accel();
    // запись в бд
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%f,%f,%f,%f)", time_request, *X_axis_msec, *Y_axis_msec, *Z_axis_msec);
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
}