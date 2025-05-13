#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_GYRO (0.135) // значение сигма для генерации
#define LIMIT (3.0)        // ограничения дипозона

// генерация случайных значений нормальным распределением
double generate_normal_gyro()
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
    z0 = u1 * factor * SIGMA_GYRO;
    z1 = u2 * factor * SIGMA_GYRO;
    if (z0 < -LIMIT * SIGMA_GYRO)
        z0 = -LIMIT * SIGMA_GYRO;
    if (z0 > LIMIT * SIGMA_GYRO)
        z0 = LIMIT * SIGMA_GYRO;
    if (z1 < -LIMIT * SIGMA_GYRO)
        z1 = -LIMIT * SIGMA_GYRO;
    if (z1 > LIMIT * SIGMA_GYRO)
        z1 = LIMIT * SIGMA_GYRO;
    spare = z1;
    has_spare = 1;
    return z0;
}

void integrade_angle(double *x_Csek, double *y_Csek, double *z_Csek, double Dt)
{
    double angle = 0;
    *x_Csek = angle + *x_Csek * Dt;
    *y_Csek = angle + *y_Csek * Dt;
    *z_Csek = angle + *z_Csek * Dt;
}

double data_gyro(double *vox_Csec, double *voy_Csec, double *voz_Csek, double ox_c, double oy_c, double oz_c, int num, double time_request, int count, double *data_roll_grad, double *data_pitch_grad, double *data_yaw_grad)
{
    double x_calibration_C;
    double y_calibration_C;
    double z_calibration_C;
    if (num == 3 || num == 4 || num == 5)
    {
        if (*vox_Csec != 0 || *voy_Csec != 0 || *voz_Csek != 0)
        {
            double x_Csek = *vox_Csec;
            double y_Csek = *voy_Csec;
            double z_Csek = *voz_Csek;
            double DT = 1;
            integrade_angle(&x_Csek, &y_Csek, &z_Csek, DT);
            x_calibration_C = x_Csek;
            y_calibration_C = y_Csek;
            z_calibration_C = z_Csek;
        }
        else
        {
            x_calibration_C = ox_c;
            y_calibration_C = oy_c;
            z_calibration_C = oz_c;
        }
    }
    else if (num == 2)
    {
        x_calibration_C = ox_c;
        y_calibration_C = oy_c;
        z_calibration_C = oz_c;
    }
    else if (num == 1)
    {
        x_calibration_C = 0;
        y_calibration_C = 0;
        z_calibration_C = 0;
    }

    x_calibration_C += generate_normal_gyro();
    y_calibration_C += generate_normal_gyro();
    z_calibration_C += generate_normal_gyro();
    *vox_Csec += generate_normal_gyro();
    *voy_Csec += generate_normal_gyro();
    *voz_Csek += generate_normal_gyro();
    *data_roll_grad = x_calibration_C;
    *data_pitch_grad = y_calibration_C;
    *data_yaw_grad = z_calibration_C;
    sqlite3 *db; // запись в бд
    char *err_msg = 0;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Gyroscopes VALUES (%f,%f,%f,%f)", time_request, *data_roll_grad, *data_pitch_grad, *data_yaw_grad);
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