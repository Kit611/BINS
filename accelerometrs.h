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

void generate_normal_accel(float *values, int n) // генерация случайных значений нормальным распределением
{
    int i = 0;
    while (i < n)
    {
        float u1, u2, s, z0, z1;
        do
        {
            u1 = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
            u2 = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
            s = u1 * u1 + u2 * u2;
        } while (s >= 1 || s == 0);

        float factor = sqrt(-2.0 * log(s) / s);
        z0 = u1 * factor * SIGMA_ACCEL;
        z1 = u2 * factor * SIGMA_ACCEL;
        if (z0 < -LIMIT * SIGMA_ACCEL)
            z0 = -LIMIT * SIGMA_ACCEL;
        if (z0 > LIMIT * SIGMA_ACCEL)
            z0 = LIMIT * SIGMA_ACCEL;
        if (z1 < -LIMIT * SIGMA_ACCEL)
            z1 = -LIMIT * SIGMA_ACCEL;
        if (z1 > LIMIT * SIGMA_ACCEL)
            z1 = LIMIT * SIGMA_ACCEL;
        values[i++] = z0;
        if (i < n)
            values[i++] = z1;
    }
}

float integrate(float acceleration, float t_start, float t_end) // функция интегрирования для получения скорости
{
    float dt = t_end - t_start;
    float initial_velocity = 0;
    float final_velocity = initial_velocity + acceleration * dt;
    return final_velocity;
}

float vx, vy, vz;

float data_accel(float *aX_m2sec, float *aY_m2sec, float *aZ_m2sec, float vx_msec, float vy_msec, float vz_msec, int num, int time_request, int count, float *Y_axis_msec, float *X_axis_msec, float *Z_axis_msec) // главная функция
{
    srand(time(NULL));
    float *values = (float *)malloc(count * sizeof(float)); // массив для сл значений
    // float *test = (float *)malloc(NUM_SAMPLES * sizeof(float));//массив для отправки итоговых значений для графика
    float X_msec, Y_msec, Z_msec;
    if (values == NULL)
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_accel(values, count);
    if (num == 3)
    {
        if (*aX_m2sec != 0 || *aY_m2sec != 0 || *aZ_m2sec != 1 || *aZ_m2sec != -1)
        {
            float t_start_x = time_request - 1, t_end_x = time_request;
            float t_start_y = time_request - 1, t_end_y = time_request;
            float t_start_z = time_request - 1, t_end_z = time_request;
            X_msec = integrate(*aX_m2sec, t_start_x, t_end_x);
            Y_msec = integrate(*aY_m2sec, t_start_y, t_end_y); // преобразование в скорость
            Z_msec = integrate(*aZ_m2sec, t_start_z, t_end_z);
            vy += Y_msec;
            if (X_msec < 13)
            {
                vx += X_msec;
            }
            else
            {
                vx = 13;
            }
            if (Z_msec < 4 && Z_msec > 0)
            {
                vz += Z_msec - 1;
            }
            else if (time_request < 53)
            {
                vz = 4;
            }
            else
            {
                vz = 0;
            }
        }
        else
        {
            X_msec = vx;
            Y_msec = vy;
            Z_msec = vz;
        }
    }
    else if (num == 2)
    {
        X_msec = vx_msec;
        Y_msec = vy_msec;
        Z_msec = vz_msec;
    }
    float x_msec = vx;
    float y_msec = vy; // для того чтобы основное число не менялось, а менялся только шум
    float z_msec = vz;
    float ax = *aX_m2sec;
    float ay = *aY_m2sec;
    float az = *aZ_m2sec;
    float aX = ax, aY = ay, aZ = az;

    if (time_request == 0)
    {
        X_msec += values[0];
        Y_msec += values[78];
        Z_msec += values[22];
        *aX_m2sec += values[47];
        *aY_m2sec += values[4];
        *aZ_m2sec += values[67];
        *X_axis_msec = X_msec;
        *Y_axis_msec = Y_msec;
        *Z_axis_msec = Z_msec;
        X_msec = x_msec;
        Y_msec = y_msec;
        Z_msec = z_msec;
    }
    else if (time_request == 1)
    {
        X_msec += values[1];
        Y_msec += values[44];
        Z_msec += values[43];
        *aX_m2sec += values[23];
        *aY_m2sec += values[5];
        *aZ_m2sec += values[65];
        *X_axis_msec = X_msec;
        *Y_axis_msec = Y_msec;
        *Z_axis_msec = Z_msec;
        X_msec = x_msec;
        Y_msec = y_msec;
        Z_msec = z_msec;
    }
    else if (time_request == 2)
    {
        X_msec += values[2];
        Y_msec += values[87];
        Z_msec += values[91];
        *aX_m2sec += values[85];
        *aY_m2sec += values[6];
        *aZ_m2sec += values[12];
        *X_axis_msec = X_msec;
        *Y_axis_msec = Y_msec;
        *Z_axis_msec = Z_msec;
        X_msec = x_msec;
        Y_msec = y_msec;
        Z_msec = z_msec;
    }
    else
    {
        for (int i = 0; i < time_request; i++)
        {
            X_msec += values[i];
            Y_msec += values[i - 1];
            Z_msec += values[i - 2];
            ax += values[i];
            ay += values[i];
            az += values[i];
            // test[i]=Z_axis_acceleration;
            *aX_m2sec = ax;
            *aY_m2sec = ay;
            *aZ_m2sec = az;
            *X_axis_msec = X_msec;
            *Y_axis_msec = Y_msec;
            *Z_axis_msec = Z_msec;
            X_msec = x_msec;
            Y_msec = y_msec;
            Z_msec = z_msec;
            ax = aX;
            ay = aY;
            az = aZ;
        }
    }
    sqlite3 *db; // запись в бд
    char *err_msg = 0;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%d,%f,%f,%f)", time_request, *X_axis_msec, *Y_axis_msec, *Z_axis_msec); // скорось по трем осям
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    free(values);
}