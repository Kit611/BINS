#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define max_height 150.0//макисмальная высота
#define min_height 10.0//минимальная высота
#define max_acceleration 176.58//максимальное ускорение
#define min_acceleration -176.58//минимальное ускорение
#define max_speed_angle 480.0//максильная угловая скорость
#define min_speed_angle 450.0//минимальная угловая скорость

int work_time=0;//вермя работы

void generate_z_acceleration(int i, int total_time, double altitude,double *az_m2_sek)//генерация ускорения по оси Z
{
    if (i < total_time - 27 && altitude < max_height && *az_m2_sek<max_acceleration)//для набора ускорения для набора высоты
    {      
        *az_m2_sek = 3.0;
    } else if (i < total_time - 27)
    {        
        *az_m2_sek = 0.0;
    } else if (altitude > min_height && *az_m2_sek>min_acceleration)
    {   
        *az_m2_sek = -1.0;
    }   
}

void generate_height(int i, int total_time, double altitude,double *az_m)
{
    if (i < total_time - 27 && altitude < max_height)//для набора высоты
    {        
        *az_m=3.0;
    } else if (i < total_time - 27)
    {        
        *az_m = 0.0;
    } else if (altitude > min_height)
    {   
        *az_m = -5.0;
    }
}

void generate_accelerometr(int i, int total_time,double *ay_m2_sek)
{
    if (*ay_m2_sek < max_acceleration && i<total_time-21)//для движения вперед
    {        
        *ay_m2_sek = 1;
    }
    else if (*ay_m2_sek > min_acceleration)
    {   
        *ay_m2_sek = -8;
    }
}

void generate_gyro(int i, int total_time, double *gyro_y_C_sec)
{
    if (*gyro_y_C_sec < max_speed_angle && i<total_time-30)//для гироскопа и движения вперед
    {        
        *gyro_y_C_sec += 0.5;
    } 
     else
    {   
        *gyro_y_C_sec -= 1.0;
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
    double x_m2_sek = 0, y_m2_sek = 0,z_m2_sek=0, z_m = min_height,az_m=min_height;
    double *x_nT = (double *)malloc(total_time * sizeof(double));
    double *y_nT = (double *)malloc(total_time * sizeof(double));
    double *z_nT = (double *)malloc(total_time * sizeof(double));
    double roll = min_speed_angle, pitch = min_speed_angle, yaw = min_speed_angle;
    double ax_m2_sek = 0, ay_m2_sek = 0, az_m2_sek = 0;
    double gyro_x_C_sec=0, gyro_y_C_sec=450, gyro_z_C_sec=0;
    if (x_nT == NULL || y_nT == NULL || z_nT == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_magnetometer(x_nT,y_nT,z_nT,total_time);
    for (int i = 0; i <= total_time; i++)//работа со значениями и запись в БД
    {
        generate_z_acceleration(i, total_time, z_m,&az_m2_sek);
        generate_height(i,total_time,z_m,&az_m);
        generate_accelerometr(i,total_time,&ay_m2_sek);
        generate_gyro(i,total_time,&gyro_y_C_sec);
        x_m2_sek += ax_m2_sek;
        y_m2_sek += ay_m2_sek;
        z_m2_sek+=az_m2_sek;
        z_m += az_m;
        if (z_m > max_height)
        {
            z_m = max_height;
            z_m2_sek = 0;
        } 
        if (z_m < min_height)
        {
            z_m = min_height;
            z_m2_sek = 0;
        }
        if(y_m2_sek>=max_acceleration)
        {
            y_m2_sek--;
        }
        if(y_m2_sek<=0)
        {
            y_m2_sek=0;
        }
        roll += gyro_x_C_sec;
        pitch = gyro_y_C_sec;
        yaw += gyro_z_C_sec;
        if(pitch>=max_speed_angle)
        {
            pitch--;
        }
        if(pitch<=min_speed_angle)
        {
            pitch=450;
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
        snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,roll, pitch, yaw,x_m2_sek,y_m2_sek,z_m2_sek,x_nT[i],y_nT[i],z_nT[i],z_m);
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