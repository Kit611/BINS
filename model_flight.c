#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define max_height_m 150.0//макисмальная высота
#define min_height_m 10.0//минимальная высота
#define up_acceleration 0.4//максимальное ускорение набора высоты
#define down_acceleration -0.3//максимальное ускорение снижения
#define max_speed 13.0//максимальная скорость
#define max_speed_angle 150//максильная угловая скорость
#define min_speed_angle 30.0//минимальная угловая скорость

int work_time=0;//вермя работы

void generate_height(int i, int total_time,double *h)
{
    if(i<3)
    {
        *h=min_height_m;
    }
    else if (i<total_time-10)
    {
        *h=max_height_m;
    }
    else
    {
        *h=min_height_m;
    }
}

void generate_accelerometr(int i, int total_time,double *ax_m2_sec,double *ay_m2_sec,double *az_m2_sec)
{
    double t=total_time-33;
    if(i<3)
    {
        *ax_m2_sec=0;
        *ay_m2_sec=0;
        *az_m2_sec=0;
    }
    else if (i<13)
    {
        *ax_m2_sec=0;
        *ay_m2_sec=0;
        *az_m2_sec=up_acceleration;
    }
    else if (i<total_time-20)
    {
        *ax_m2_sec=0;
        *ay_m2_sec=max_speed/t;
        *az_m2_sec=0;
    }
    else if(i<total_time-10)
    {
        *ax_m2_sec=0;
        *ay_m2_sec=0;
        *az_m2_sec=down_acceleration;
    }
    else
    {
        *ax_m2_sec=0;
        *ay_m2_sec=0;
        *az_m2_sec=0;
    }
}

void generate_gyro(int i, int total_time, double *gyro_x_C_sec,double *gyro_y_C_sec,double *gyro_z_C_sec)
{
    if(i<13)
    {
        *gyro_x_C_sec=0;
        *gyro_y_C_sec=0;
        *gyro_z_C_sec=0;
    }  
    else if (i<total_time-21)
    {
        *gyro_x_C_sec=0;
        *gyro_y_C_sec=max_speed_angle;
        *gyro_z_C_sec=0;
    }
    else if(i<total_time-20)
    {
        *gyro_x_C_sec=0;
        *gyro_y_C_sec=0;
        *gyro_z_C_sec=0;
    }
    else if(i<total_time-10)
    {
        *gyro_x_C_sec=0;
        *gyro_y_C_sec=-max_speed_angle;
        *gyro_z_C_sec=0;
    }
    else
    {
        *gyro_x_C_sec=0;
        *gyro_y_C_sec=0;
        *gyro_z_C_sec=0;
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
    double ax_m2_sec,ay_m2_sec,az_m2_sec;
    double gyro_x_C_sec,gyro_y_C_sec,gyro_z_C_sec;
    double h_m;
    double m_kg=0.249;
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
        generate_accelerometr(i,total_time,&ax_m2_sec,&ay_m2_sec,&az_m2_sec);
        generate_gyro(i,total_time,&gyro_x_C_sec,&gyro_y_C_sec,&gyro_z_C_sec);
        generate_height(i,total_time,&h_m);
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
            sqlite3_close(db);
            return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,gyro_x_C_sec, gyro_y_C_sec, gyro_z_C_sec,ax_m2_sec,ay_m2_sec,az_m2_sec,x_nT[i],y_nT[i],z_nT[i],h_m);
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