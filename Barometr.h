#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_BAR 0.025 //значение сигма для генерации
#define LIMIT 3.0   //ограничение диапозона

void generate_normal_bar(double *values, int n)//генерация случайных значений нормальным распределением
{
    int i = 0;
    while (i < n)
    {
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

        if (z0 < -LIMIT * SIGMA_BAR) z0 = -LIMIT * SIGMA_BAR;
        if (z0 > LIMIT * SIGMA_BAR) z0 = LIMIT * SIGMA_BAR;
        if (z1 < -LIMIT * SIGMA_BAR) z1 = -LIMIT * SIGMA_BAR;
        if (z1 > LIMIT * SIGMA_BAR) z1 = LIMIT * SIGMA_BAR;

        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

double data_bar(double h_m,double sys_er, int time_request,int NUM_SAMPLES)//главная функция
{        
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для сл значений
    // double *Bar = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для отправки итоговых значений для графика
    if (values == NULL ) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_bar(values, NUM_SAMPLES);
    double real_h_m=h_m;
    double P_mbar;
    double p_mbar;
    const double P_0_mbar=1013.25;//давление на станадартной высоте в милибарах
    const int H_m=8400;//стандартная высота
    double stepen=real_h_m/8400;
    double e=exp(-stepen);
    P_mbar=P_0_mbar*e;//перевод в милибары
    p_mbar=P_mbar;
    double data_request_mbar;
    P_mbar+=sys_er; //добавление системной ошибки
    if(time_request==0)
    {
        P_mbar+=values[10];
        data_request_mbar=P_mbar; 
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {
            P_mbar+=values[i-1];
            data_request_mbar=P_mbar; 
        }
    }
    sqlite3 *db;//запись в бд
    char *err_msg=0;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc !=SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }   
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Barometr VALUES (%d,%f,%f,%f);", time_request, real_h_m, p_mbar, P_mbar);//давление в милибарах с шумом
    rc=sqlite3_exec(db,sql,0,0,&err_msg);
    if(rc!=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    free(values);
    return data_request_mbar;
}