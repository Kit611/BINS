#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ALTITUDE 150.0
#define LANDING_ALTITUDE 10.0

int work_time=0;

void control_inputs(int i, int total_time, double altitude, 
                    double *ax, double *ay, double *az, 
                    double *gyro_x, double *gyro_y, double *gyro_z)
{
    if (i < total_time - 10 && altitude < MAX_ALTITUDE)
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
    *gyro_x = 0.05 * sin(i / 5.0);
    *gyro_y = 0.05 * cos(i / 4.0);
    *gyro_z = 0.03 * sin(i / 6.0);
}

double generate_magnetometer_value()
{
    double min_micros = 25.0;
    double max_micros = 65.0;
    double microtesla = min_micros + (rand() / (RAND_MAX / (max_micros - min_micros)));   
    return microtesla / 100000.0;
}

int get_time()
{
    return work_time;
}

int flight()
{
    int total_time;
    printf("Введите время моделирования (секунды): ");
    scanf("%d", &total_time);
    work_time=total_time;
    double x = 0, y = 0, z = LANDING_ALTITUDE;
    double vx = 0, vy = 0, vz = 0;
    double roll = 0, pitch = 0, yaw = 0;
    for (int i = 0; i <= total_time; i++)
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
        pitch += gyro_y;
        yaw += gyro_z;
        double mx, my, mz;
        mx=generate_magnetometer_value();
        my=generate_magnetometer_value();
        mz=generate_magnetometer_value();
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i, roll, pitch, yaw,vx,vy,vz,mx,my,mz,z);
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