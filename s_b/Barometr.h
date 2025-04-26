#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_BAR (0.025) // значение сигма для генерации
#define LIMIT (3.0)       // ограничение диапозона

// генерация случайных значений нормальным распределением
// void generate_normal_bar(double *values, int n)
double generate_normal_bar()
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
    //     values[i++] = z0;
    //     if (i < n)
    //         values[i++] = z1;
    // }
}

double data_bar(double h_m, double sys_er, double time_request, int count)
{
    srand(time(NULL));
    // double *values = (double *)malloc(count * sizeof(double)); // массив для сл значенийs
    // double *Bar = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для отправки итоговых значений для графика
    // if (values == NULL)
    // {
    //     fprintf(stderr, "Ошибка выделения памяти\n");
    //     return 1;
    // }
    double noise = generate_normal_bar();
    double real_h_m = h_m;
    double P_mbar;
    double p_mbar;
    const double P_0_mbar = 1013.25; // давление на станадартной высоте в милибарах
    const int H_m = 8400;            // стандартная высота
    double stepen = real_h_m / 8400;
    double e = exp(-stepen);
    P_mbar = P_0_mbar * e; // перевод в милибары
    p_mbar = P_mbar;
    P_mbar += sys_er + noise; // добавление системной ошибки и шума
    sqlite3 *db;              // запись в бд
    char *err_msg = 0;
    int rc = sqlite3_open("Logs.db", &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Barometr VALUES (%f,%f,%f,%f);", time_request, real_h_m, p_mbar, P_mbar);
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    return P_mbar;
}