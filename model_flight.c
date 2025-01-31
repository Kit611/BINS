#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ALTITUDE 150.0//макисмальная высота
#define LANDING_ALTITUDE 10.0//минимальная высота

int work_time=0;//вермя работы

void control_inputs(int i, int total_time, double altitude,double *ax_m2_sek, double *ay_m2_sek, double *az_m2_sek, double *gyro_x_C_sec, double *gyro_y_C_sec, double *gyro_z_C_sec)//контроль и генерация значений для полета
{
    if (i < total_time - 10 && altitude < MAX_ALTITUDE)//для акселерометра
    {        
        *az_m2_sek = 3.0;
    } else if (i < total_time - 10)
    {        
        *az_m2_sek = 0.0;
        *ay_m2_sek = 1.0;
    } else if (altitude > LANDING_ALTITUDE)
    {   
        *ay_m2_sek=-8.0;
        *az_m2_sek = -3.0;
    }
    if(*gyro_y_C_sec<30.0 && i<total_time-30)//для гироскопа
    {
        *gyro_x_C_sec = 0.0;
        *gyro_z_C_sec = 0.0;
        *gyro_y_C_sec+=1;
    }
    else
    {
        *gyro_y_C_sec-=1;
    }
}


//генерация значений для магнетомерта
void generate_magnetometer(double *x_nT, double *y_nT, double *z_nT,int count)
{
    for(int i=0;i<count;i++)
    {
        double angle = (double)i / count * 2 * M_PI;
        x_nT[i] = cos(angle);
        y_nT[i] = sin(angle);
        z_nT[i] = 0.5 * cos(angle / 2);
    }
}

int get_time()//передача времени
{
    return work_time;
}

int flight(double *lon,double *lat)
{
    int total_time;
    printf("Введите время моделирования (секунды): ");
    scanf("%d", &total_time);
    *lon=30.26774253847127;
    *lat=59.802977330951;
    work_time=total_time;
    double x_m2_sek = 0, y_m2_sek = 0, z_m = LANDING_ALTITUDE;
    double vx_m2_sek = 0, vy_m2_sek = 0, vz_m2_sek = 0;
    double roll = 0, pitch = 0, yaw = 0;
    double *x_nT = (double *)malloc(total_time * sizeof(double));
    double *y_nT = (double *)malloc(total_time * sizeof(double));
    double *z_nT = (double *)malloc(total_time * sizeof(double));
    if (x_nT == NULL || y_nT == NULL || z_nT == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_magnetometer(x_nT,y_nT,z_nT,total_time);
    for (int i = 0; i <= total_time; i++)//работа со значениями и запись в БД
    {
        double ax_m2_sek = 0, ay_m2_sek = 0, az_m2_sek = 0;
        double gyro_x_C_sec, gyro_y_C_sec, gyro_z_C_sec;
        control_inputs(i, total_time, z_m, &ax_m2_sek, &ay_m2_sek, &az_m2_sek, &gyro_x_C_sec, &gyro_y_C_sec, &gyro_z_C_sec);
        vx_m2_sek += ax_m2_sek;
        vy_m2_sek += ay_m2_sek;
        vz_m2_sek += az_m2_sek;
        x_m2_sek += vx_m2_sek;
        y_m2_sek += vy_m2_sek;
        z_m += vz_m2_sek;
        if (z_m > MAX_ALTITUDE)
        {
            z_m = MAX_ALTITUDE;
            vz_m2_sek = 0;
        } 
        if (z_m < LANDING_ALTITUDE)
        {
            z_m = LANDING_ALTITUDE;
            vz_m2_sek = 0;
        }
        if(vy_m2_sek<0)
        {
            vy_m2_sek=0;
        }
        roll += gyro_x_C_sec;
        pitch = gyro_y_C_sec;
        yaw += gyro_z_C_sec;
        if(pitch>=30.0)
        {
            pitch--;
        }
        if(pitch<=0)
        {
            pitch=0;
        }
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
            sqlite3_close(db);
            return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,roll, pitch, yaw,vx_m2_sek,vy_m2_sek,vz_m2_sek,x_nT[i],y_nT[i],z_nT[i],z_m);
        rc=sqlite3_exec(db,sql,0,0,&err_msg);
        if(rc!=SQLITE_OK)
        {
            printf ("SQL error: %s\n",err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_close(db);
    }
    free(x_nT);
    free(y_nT);
    free(z_nT);
    return 0;
}