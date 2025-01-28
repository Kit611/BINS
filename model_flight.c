#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ALTITUDE 150.0//макисмальная высота
#define LANDING_ALTITUDE 10.0//минимальная высота
#define UT_TO_MG 10.0

int work_time=0;//вермя работы

void control_inputs(int i, int total_time, double altitude, 
                    double *ax, double *ay, double *az, 
                    double *gyro_x, double *gyro_y, double *gyro_z)//контроль и генерация значений для полета
{
    if (i < total_time - 10 && altitude < MAX_ALTITUDE)//для акселерометра
    {        
        *az = 3.0;
    } else if (i < total_time - 10)
    {        
        *az = 0.0;
        *ay = 1.0;
    } else if (altitude > LANDING_ALTITUDE)
    {   
        *ay=-8.0;
        *az = -3.0;
    }
    if(*gyro_y<30.0 && i<total_time-30)//для гироскопа
    {
        *gyro_x = 0.0;
        *gyro_z = 0.0;
        *gyro_y+=1;
    }
    else
    {
        *gyro_y-=1;
    }
}


//генерация значений для магнетомерта
void compute_magnetometer(double pitch, double B0, double *Bx, double *By, double *Bz)
{
    double pitch_rad = pitch * M_PI / 180.0;
    *Bx = B0 * cos(pitch_rad);
    *By = 0;
    *Bz = B0 * sin(pitch_rad);
    *Bx *= UT_TO_MG;
    *By *= UT_TO_MG;
    *Bz *= UT_TO_MG;
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
    double x = 0, y = 0, z = LANDING_ALTITUDE;
    double vx = 0, vy = 0, vz = 0;
    double roll = 0, pitch = 0, yaw = 0;
    int ch=-3;
    for (int i = 0; i <= total_time; i++)//работа со значениями и запись в БД
    {
        double ax = 0, ay = 0, az = 0;
        double gyro_x, gyro_y, gyro_z;
        control_inputs(i, total_time, z, &ax, &ay, &az, &gyro_x, &gyro_y, &gyro_z);
        vx += ax;
        vy += ay;
        vz += az;
        x += vx;
        y += vy;
        z += vz;
        if (z > MAX_ALTITUDE)
        {
            z = MAX_ALTITUDE;
            vz = 0;
        } 
        if (z < LANDING_ALTITUDE)
        {
            z = LANDING_ALTITUDE;
            vz = 0;
        }
        if(vy<0)
        {
            vy=0;
        }
        roll += gyro_x;
        pitch = gyro_y;
        yaw += gyro_z;
        if(pitch>=30.0)
        {
            pitch--;
        }
        if(pitch<=0)
        {
            pitch=0;
        }
        double mx, my, mz;
        double B0=50.0;
        compute_magnetometer(pitch,B0,&mx,&my,&mz);
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,roll, pitch, yaw,vx,vy,vz,mx,my,mz,z);
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
    return 0;
}