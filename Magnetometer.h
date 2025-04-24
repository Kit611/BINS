#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_MAG (0.22)          // значение сигма для генерации
#define LIMIT (3.0)               // ограничение диапозона
#define DEG_TO_RAD (M_PI / 180.0) // для перевода в радианы

// генерация случайных значений нормальным распределением
// void generate_normal_mag(double *values, int n)
double generate_normal_mag()
{
    // int i = 0;
    // while (i < n)
    // {
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
    z0 = u1 * factor * SIGMA_MAG;
    z1 = u2 * factor * SIGMA_MAG;
    if (z0 < -LIMIT * SIGMA_MAG)
        z0 = -LIMIT * SIGMA_MAG;
    if (z0 > LIMIT * SIGMA_MAG)
        z0 = LIMIT * SIGMA_MAG;
    if (z1 < -LIMIT * SIGMA_MAG)
        z1 = -LIMIT * SIGMA_MAG;
    if (z1 > LIMIT * SIGMA_MAG)
        z1 = LIMIT * SIGMA_MAG;
    spare = z1;
    has_spare = 1;
    return z0;
    //     values[i++] = z0;
    //     if (i < n)
    //         values[i++] = z1;
    // }
}

// расчет матрицы поворота
void compute_rotation_matrix(double yaw, double pitch, double roll, double R[3][3])
{
    double cy = cos(yaw * DEG_TO_RAD), sy = sin(yaw * DEG_TO_RAD);
    double cp = cos(pitch * DEG_TO_RAD), sp = sin(pitch * DEG_TO_RAD);
    double cr = cos(roll * DEG_TO_RAD), sr = sin(roll * DEG_TO_RAD);
    R[0][0] = cy * cp;
    R[0][1] = cy * sp * sr - sy * cr;
    R[0][2] = cy * sp * cr + sy * sr;
    R[1][0] = sy * cp;
    R[1][1] = sy * sp * sr + cy * cr;
    R[1][2] = sy * sp * cr - cy * sr;
    R[2][0] = -sp;
    R[2][1] = cp * sr;
    R[2][2] = cp * cr;
}

// Применение матрицы поворота к вектору магнитного поля
void transform_magnetic_field(double R[3][3], double *x_G, double *y_G, double *z_G, double Bx_G, double By_G, double Bz_G)
{
    *x_G = R[0][0] * Bx_G + R[0][1] * By_G + R[0][2] * Bz_G;
    *y_G = R[1][0] * Bx_G + R[1][1] * By_G + R[1][2] * Bz_G;
    *z_G = R[2][0] * Bx_G + R[2][1] * By_G + R[2][2] * Bz_G;
}

// Функция для вычисления магнитного склонения
double calculate_declination(double mx, double my)
{
    double declination = atan2(my, mx);
    return declination * (180 / M_PI); // Возвращаем угол склонения в градусах
}

// Функция для вычисления магнитного наклонения
double calculate_inclination(double mx, double my, double mz)
{
    double b_hor = sqrt((mx * mx) + (my * my));
    double inclination = atan2(mz, b_hor);
    return inclination * (180 / M_PI); // Возвращаем угол наклонения в градусах
}

// магнитное направление
double mag_napr(double x, double y)
{
    return atan2(y, x);
}

double data_mag(double Bx_G, double By_G, double Bz_G, double roll_C, double pitch_C, double yaw_C, double time_request, int count, double *data_x_mG, double *data_y_mG, double *data_z_mG, double *declination_c, double *inclination_c)
{
    srand(time(NULL));
    double R[3][3];
    double x_G, y_G, z_G;
    compute_rotation_matrix(yaw_C, pitch_C, roll_C, R);              // расчет поворотной матрицы
    transform_magnetic_field(R, &x_G, &y_G, &z_G, Bx_G, By_G, Bz_G); // изменение значения в зависимости от углов ориентации
    double x_mG = x_G * 1000;
    double y_mG = y_G * 1000;
    double z_mG = z_G * 1000;
    double X_mG = x_mG;
    double Y_mG = y_mG; // для того чтобы основное число не менялось, а менялся только шум
    double Z_mG = z_mG;
    *data_x_mG += generate_normal_mag();
    *data_y_mG += generate_normal_mag();
    *data_z_mG += generate_normal_mag();
    double data_xmG = abs(*data_x_mG);
    double data_ymG = abs(*data_y_mG);
    double data_zmG = abs(*data_z_mG);
    double declination_grad = calculate_declination(data_xmG, data_ymG);           // расчет магнитного склонения
    double inclination_grad = calculate_inclination(data_xmG, data_ymG, data_zmG); // расчет магнитного наклонения
    *declination_c = declination_grad;
    *inclination_c = inclination_grad;
    double angle_dir_rad = mag_napr(data_xmG, data_ymG); // расчет магнитного направления
    double angle_dir_grad = angle_dir_rad * (180 / M_PI);
    double true_dir_grad;
    if (declination_grad > 0) // расчет истонного курса
    {
        true_dir_grad = angle_dir_grad + declination_grad;
    }
    else
    {
        true_dir_grad = angle_dir_grad - declination_grad;
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
    snprintf(sql, sizeof(sql), "INSERT INTO Magnetometer VALUES (%f,%f,%f,%f,%f,%f,%f)", time_request, *data_x_mG, *data_y_mG, *data_z_mG, declination_grad, inclination_grad, true_dir_grad);
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