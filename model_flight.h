#pragma once
#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
//ми 8мт "Метео"
#define Time_work 100//время работы минуты(секунды)
#define max_height 4500//максимальная высота(150)
#define min_height 10//минимальная высота
#define max_speed 64//максимальная горизонтальная сокрость(64 м/с, 230км/ч)13
#define up_speed 6//скорость набора высоты(6м/с)4
#define down_speed 6//скорость посадки(6м/с)3
#define down_angle -30//тангаж
#define up_angle 30
#define direction Notrh//курс полета
int time_sec;

int get_time()//передача времени
{
    return time_sec;
}

int flight(int num_model)
{
    time_sec=Time_work*60;
    const double g_m2Sec=1;
    double h_m=0;
    double X=0;
    double Y=0;
    double ox_c=0,oy_c=0,oz_c=0;
    double vx_msec=0,vy_msec=0,vz_msec=0;
    double vox_csec=0,voy_csec=0,voz_csec=0;
    double ax_m2sec=0,ay_m2sec=0,az_m2sec=g_m2Sec;
    double Bx_G=0.14451,By_G=0.03056,Bz_G=0.50904;
    switch (num_model)//генерация значения в зависимости от модели полета
    {
    case 1:
        printf("Модель висения.\n");
        for(int i=0;i<time_sec;i++)
        {
            h_m=max_height;
            sqlite3 *db;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            char sql[256];
            snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,X,Y,h_m,ox_c,oy_c,oz_c,vx_msec,vy_msec,vz_msec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,Bx_G,By_G,Bz_G);
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
        break;
    case 2:
        printf("Модель линейного полета.\n");
        for(int i=0;i<time_sec;i++)
        {
            h_m=max_height;
            vx_msec=max_speed;
            oy_c=down_angle;
            X+=vx_msec;
            sqlite3 *db;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            char sql[256];
            snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,X,Y,h_m,ox_c,oy_c,oz_c,vx_msec,vy_msec,vz_msec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,Bx_G,By_G,Bz_G);
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
        break;
    case 3:
        printf("Модель: взлет->движениe->посадка.\n");
        h_m=min_height;
        for(int i=0;i<time_sec;i++)
        {
            if(i>time_sec-5900 && i<time_sec-5650)
            {
                az_m2sec=1.06;
                if(vz_msec<up_speed)
                {
                    vz_msec+=az_m2sec-g_m2Sec;
                }
                else
                {
                    az_m2sec=-g_m2Sec;
                    vz_msec=up_speed;
                }
                
                if(h_m<max_height)
                {
                    h_m+=vz_msec;    
                }
                else
                {
                    h_m=max_height;
                }
            }
            else if (i>time_sec-5650 && i<time_sec-5648)
            {
                voy_csec=-30;                
            }
            else if(i>time_sec-5649 && i<time_sec-1154)
            {
                oy_c=down_angle;
                voy_csec=0;
                ax_m2sec=0.64;
                Bx_G-=0.0000000256;
                Bz_G+=0.0000000256;
                if(vx_msec<max_speed)
                {
                    vx_msec+=ax_m2sec;
                }
                else
                {
                    ax_m2sec=0;
                    vx_msec=max_speed;
                }                
                if(h_m<max_height)
                {
                    h_m+=vz_msec;   
                }
                else
                {
                    vz_msec=0;
                    h_m=max_height;
                }
                X+=vx_msec;
            }            
            else if (i>time_sec-1154 && i<time_sec-1152)
            {
                voy_csec=0;
                oy_c=voy_csec;
            }
            else if (i>time_sec-1152 && i<time_sec-1150)
            {
                voy_csec=30;
            }
            else if (i>time_sec-1151 && i<time_sec-1050)
            {
                oy_c=up_angle;
                voy_csec=0;
                ax_m2sec=-0.64;
                if(vx_msec>0)
                {
                    vx_msec+=ax_m2sec;
                }
                else if(vx_msec<=0)
                {                
                    vx_msec=0;
                }
                if (vx_msec==0)
                {
                    ax_m2sec=0;
                }                
                X+=vx_msec;
            }
            else
            {
                ax_m2sec=0;
                oy_c=0;
                az_m2sec=-0.4;
                if(vz_msec<down_speed)
                {
                    vz_msec+=az_m2sec+g_m2Sec;
                }
                else
                {
                    az_m2sec=g_m2Sec;
                    vz_msec=down_speed;
                }                
                if(h_m>min_height)
                {
                    h_m-=vz_msec;
                }
                else
                {
                    vz=0;
                    h_m=min_height;
                }
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
            snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,X,Y,h_m,ox_c,oy_c,oz_c,vx_msec,vy_msec,vz_msec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,Bx_G,By_G,Bz_G);
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
        break;
    case 4:
        printf("Модель: движение с поворотом.\n");
        h_m=min_height;
        for(int i=0;i<time_sec;i++)
        {
            sqlite3 *db;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            char sql[256];
            snprintf(sql, sizeof(sql), "INSERT INTO model_flight VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",i,X,Y,h_m,ox_c,oy_c,oz_c,vx_msec,vy_msec,vz_msec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,Bx_G,By_G,Bz_G);
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
        break;
    default:
        printf("Такой модели полета нет");
        break;
    }
    return 0;
}