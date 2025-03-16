#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SIGMA_GYRO (0.135)//значение сигма для генерации
#define LIMIT (3.0)   //ограничения дипозона 

void generate_normal_gyro(double *values, int n)//генерация случайных значений нормальным распределением
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
        z0 = u1 * factor * SIGMA_GYRO;
        z1 = u2 * factor * SIGMA_GYRO;
        if (z0 < -LIMIT * SIGMA_GYRO) z0 = -LIMIT * SIGMA_GYRO;
        if (z0 > LIMIT * SIGMA_GYRO) z0 = LIMIT * SIGMA_GYRO;
        if (z1 < -LIMIT * SIGMA_GYRO) z1 = -LIMIT * SIGMA_GYRO;
        if (z1 > LIMIT * SIGMA_GYRO) z1 = LIMIT * SIGMA_GYRO;
        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

void integrade_angle(double *x_Csek,double *y_Csek,double *z_Csek,double Dt)
{
    double angle=0;
    *x_Csek=angle+*x_Csek*Dt;
    *y_Csek=angle+*y_Csek*Dt;
    *z_Csek=angle+*z_Csek*Dt;
}

double data_gyro(double *vox_Csec,double *voy_Csec,double *voz_Csek,double ox_c, double oy_c,double oz_c,int num,int time_request,int count,double *data_roll_grad,double *data_pitch_grad,double * data_yaw_grad)//главная функция
{
    srand(time(NULL));
    double *values = (double *)malloc(count * sizeof(double));//массив для сл значений
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_gyro(values, count);
    double x_calibration_C;
    double y_calibration_C;
    double z_calibration_C;
    double vx=*vox_Csec;
    double vy=*voy_Csec;
    double vz=*voz_Csek;
    double Vx=vx,Vy=vy,Vz=vz;
    if(num==3)
    {
        if(*vox_Csec!=0 || *voy_Csec!=0 || *voz_Csek!=0)
        {
        double x_Csek=*vox_Csec;
        double y_Csek=*voy_Csec;
        double z_Csek=*voz_Csek;
        double DT=1;
        integrade_angle(&x_Csek,&y_Csek,&z_Csek,DT);
        x_calibration_C=x_Csek;
        y_calibration_C=y_Csek;
        z_calibration_C=z_Csek;
        }
        else
        {
            x_calibration_C=ox_c;
            y_calibration_C=oy_c;
            z_calibration_C=oz_c;
        }
    }
    else if(num==2)
    {
        x_calibration_C=ox_c;
        y_calibration_C=oy_c;
        z_calibration_C=oz_c;
    }
    else if (num==1)
    {
        x_calibration_C=0;
        y_calibration_C=0;
        z_calibration_C=0;
    }
    
    double Roll_Csec=x_calibration_C;
    double Pitch_Csec=y_calibration_C;     //для того чтобы основное число не менялось, а менялся только шум
    double Yaw_Csec=x_calibration_C;
    if(time_request==0)
    {
        x_calibration_C+=values[0];
        y_calibration_C+=values[33];
        z_calibration_C+=values[65];
        *vox_Csec+=values[31];
        *voy_Csec+=values[12];
        *voz_Csek+=values[98];
        *data_roll_grad=x_calibration_C;
        *data_pitch_grad=y_calibration_C;
        *data_yaw_grad=z_calibration_C; 
        x_calibration_C=Roll_Csec;
        y_calibration_C=Pitch_Csec;
        z_calibration_C=Yaw_Csec;  
    }
    else if(time_request==1)
    {
        x_calibration_C+=values[1];
        y_calibration_C+=values[43];
        z_calibration_C+=values[86];
        *vox_Csec+=values[8];
        *voy_Csec+=values[73];
        *voz_Csek+=values[10];
        *data_roll_grad=x_calibration_C;
        *data_pitch_grad=y_calibration_C;
        *data_yaw_grad=z_calibration_C;  
        x_calibration_C=Roll_Csec;
        y_calibration_C=Pitch_Csec;
        z_calibration_C=Yaw_Csec; 
    }
    else if (time_request==2)
    {
        x_calibration_C+=values[3];
        y_calibration_C+=values[21];
        z_calibration_C+=values[62];
        *vox_Csec+=values[5];
        *voy_Csec+=values[72];
        *voz_Csek+=values[22];
        *data_roll_grad=x_calibration_C;
        *data_pitch_grad=y_calibration_C;
        *data_yaw_grad=z_calibration_C; 
        x_calibration_C=Roll_Csec;
        y_calibration_C=Pitch_Csec;
        z_calibration_C=Yaw_Csec;  
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {        
            x_calibration_C+=values[i];
            y_calibration_C+=values[i-1];
            z_calibration_C+=values[i-2];
            vx+=values[i];
            vy+=values[i];
            vz+=values[i];
            *vox_Csec=vx;
            *voy_Csec=vy;
            *voz_Csek=vz;
            *data_roll_grad=x_calibration_C;
            *data_pitch_grad=y_calibration_C;
            *data_yaw_grad=z_calibration_C;        
            x_calibration_C=Roll_Csec;
            y_calibration_C=Pitch_Csec;
            z_calibration_C=Yaw_Csec;
            vx=Vx;
            vy=Vy;
            vz=Vz;
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
    snprintf(sql, sizeof(sql), "INSERT INTO Gyroscopes VALUES (%d,%f,%f,%f)", time_request, *data_roll_grad, *data_pitch_grad, *data_yaw_grad);//угол наклона
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
}