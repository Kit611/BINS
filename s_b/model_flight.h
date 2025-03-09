#pragma once
#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//ми 8мт "Метео"
#define TIME_WORK (100)//время работы минуты(секунды)
#define MAX_HEIGHT 4500//максимальная высота(150)
#define MIN_HEIGHT 10//минимальная высота
#define MAX_SPEED 64//максимальная горизонтальная сокрость(64 м/с, 230км/ч)13
#define UP_SPEED 6//скорость набора высоты(6м/с)4
#define DOWN_SPEED 6//скорость посадки(6м/с)3
#define DOWN_ANGLE -30//тангаж
#define UP_ANGLE 30

int time_sec;

int get_time()//передача времени
{
    return time_sec;
}

int flight(int num_model)
{
    time_sec=TIME_WORK*60;
    char direction[15]="North";
    const double g_m2Sec=1;
    double h_m=0;
    double X=0;
    double Y=0;
    double ox_c=0,oy_c=0,oz_c=0;
    double vx_msec=0,vy_msec=0,vz_msec=0;
    double vox_csec=0,voy_csec=0,voz_csec=0;
    double ax_m2sec=0,ay_m2sec=0,az_m2sec=g_m2Sec;
    double Bx_G=0.14451,By_G=0.03056,Bz_G=0.50904;
    const char *sql="INSERT INTO model_flight VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20)";
    switch (num_model)//генерация значения в зависимости от модели полета
    {
    case 1:
        printf("Модель висения.\n");
        for(int i=0;i<time_sec;i++)
        {
            h_m=MAX_HEIGHT;
            sqlite3 *db;
            sqlite3_stmt *res;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
            if(rc==SQLITE_OK)
            {
                sqlite3_bind_int(res,1,i);
                sqlite3_bind_double(res,2,X);
                sqlite3_bind_double(res,3,Y);
                sqlite3_bind_double(res,4,h_m);
                sqlite3_bind_text(res,5,direction,-1,SQLITE_STATIC);
                sqlite3_bind_double(res,6,ox_c);
                sqlite3_bind_double(res,7,oy_c);
                sqlite3_bind_double(res,8,oz_c);
                sqlite3_bind_double(res,9,vx_msec);
                sqlite3_bind_double(res,10,vy_msec);
                sqlite3_bind_double(res,11,vz_msec);
                sqlite3_bind_double(res,12,vox_csec);
                sqlite3_bind_double(res,13,voy_csec);
                sqlite3_bind_double(res,14,voz_csec);
                sqlite3_bind_double(res,15,ax_m2sec);
                sqlite3_bind_double(res,16,ay_m2sec);
                sqlite3_bind_double(res,17,az_m2sec);
                sqlite3_bind_double(res,18,Bx_G);
                sqlite3_bind_double(res,19,By_G);
                sqlite3_bind_double(res,20,Bz_G);
                while (rc=sqlite3_step(res)==SQLITE_ROW){}                
            }
            else
            {
                printf ("SQL error: %s\n",err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                return 1;
            }
            sqlite3_finalize(res);
            sqlite3_close(db);
        }
        break;
    case 2:
        printf("Модель линейного полета.\n");
        for(int i=0;i<time_sec;i++)
        {
            h_m=MAX_HEIGHT;
            vx_msec=MAX_SPEED;
            oy_c=DOWN_ANGLE;
            Bx_G-=0.0000000256;
            Bz_G+=0.0000000256;
            X+=vx_msec;
            sqlite3 *db;
            sqlite3_stmt *res;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
            if(rc==SQLITE_OK)
            {
                sqlite3_bind_int(res,1,i);
                sqlite3_bind_double(res,2,X);
                sqlite3_bind_double(res,3,Y);
                sqlite3_bind_double(res,4,h_m);
                sqlite3_bind_text(res,5,direction,-1,SQLITE_STATIC);
                sqlite3_bind_double(res,6,ox_c);
                sqlite3_bind_double(res,7,oy_c);
                sqlite3_bind_double(res,8,oz_c);
                sqlite3_bind_double(res,9,vx_msec);
                sqlite3_bind_double(res,10,vy_msec);
                sqlite3_bind_double(res,11,vz_msec);
                sqlite3_bind_double(res,12,vox_csec);
                sqlite3_bind_double(res,13,voy_csec);
                sqlite3_bind_double(res,14,voz_csec);
                sqlite3_bind_double(res,15,ax_m2sec);
                sqlite3_bind_double(res,16,ay_m2sec);
                sqlite3_bind_double(res,17,az_m2sec);
                sqlite3_bind_double(res,18,Bx_G);
                sqlite3_bind_double(res,19,By_G);
                sqlite3_bind_double(res,20,Bz_G);
                while (rc=sqlite3_step(res)==SQLITE_ROW){}                
            }
            else
            {
                printf ("SQL error: %s\n",err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                return 1;
            }
            sqlite3_finalize(res);
            sqlite3_close(db);
        }
        break;
    case 3:
        printf("Модель: взлет->движениe->посадка.\n");
        h_m=MIN_HEIGHT;
        for(int i=0;i<time_sec;i++)
        {
            if(i>time_sec-5900 && i<time_sec-5650)
            {
                az_m2sec=1.06;
                if(vz_msec<UP_SPEED)
                {
                    vz_msec+=az_m2sec-g_m2Sec;
                }
                else
                {
                    az_m2sec=-g_m2Sec;
                    vz_msec=UP_SPEED;
                }
                
                if(h_m<MAX_HEIGHT)
                {
                    h_m+=vz_msec;    
                }
                else
                {
                    h_m=MAX_HEIGHT;
                }
            }
            else if (i>time_sec-5650 && i<time_sec-5648)
            {
                voy_csec=DOWN_ANGLE;                
            }
            else if(i>time_sec-5649 && i<time_sec-1154)
            {
                oy_c=DOWN_ANGLE;
                voy_csec=0;
                ax_m2sec=0.64;
                Bx_G-=0.0000000256;
                Bz_G+=0.0000000256;
                if(vx_msec<MAX_SPEED)
                {
                    vx_msec+=ax_m2sec;
                }
                else
                {
                    ax_m2sec=0;
                    vx_msec=MAX_SPEED;
                }                
                if(h_m<MAX_HEIGHT)
                {
                    h_m+=vz_msec;   
                }
                else
                {
                    vz_msec=0;
                    h_m=MAX_HEIGHT;
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
                voy_csec=UP_ANGLE;
            }
            else if (i>time_sec-1151 && i<time_sec-1050)
            {
                oy_c=UP_ANGLE;
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
                if(vz_msec<DOWN_SPEED)
                {
                    vz_msec+=az_m2sec+g_m2Sec;
                }
                else
                {
                    az_m2sec=g_m2Sec;
                    vz_msec=DOWN_SPEED;
                }                
                if(h_m>MIN_HEIGHT)
                {
                    h_m-=vz_msec;
                }
                else
                {
                    vz=0;
                    h_m=MIN_HEIGHT;
                }
            }            
            sqlite3 *db;
            sqlite3_stmt *res;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
            if(rc==SQLITE_OK)
            {
                sqlite3_bind_int(res,1,i);
                sqlite3_bind_double(res,2,X);
                sqlite3_bind_double(res,3,Y);
                sqlite3_bind_double(res,4,h_m);
                sqlite3_bind_text(res,5,direction,-1,SQLITE_STATIC);
                sqlite3_bind_double(res,6,ox_c);
                sqlite3_bind_double(res,7,oy_c);
                sqlite3_bind_double(res,8,oz_c);
                sqlite3_bind_double(res,9,vx_msec);
                sqlite3_bind_double(res,10,vy_msec);
                sqlite3_bind_double(res,11,vz_msec);
                sqlite3_bind_double(res,12,vox_csec);
                sqlite3_bind_double(res,13,voy_csec);
                sqlite3_bind_double(res,14,voz_csec);
                sqlite3_bind_double(res,15,ax_m2sec);
                sqlite3_bind_double(res,16,ay_m2sec);
                sqlite3_bind_double(res,17,az_m2sec);
                sqlite3_bind_double(res,18,Bx_G);
                sqlite3_bind_double(res,19,By_G);
                sqlite3_bind_double(res,20,Bz_G);
                while (rc=sqlite3_step(res)==SQLITE_ROW){}                
            }
            else
            {
                printf ("SQL error: %s\n",err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                return 1;
            }
            sqlite3_finalize(res);
            sqlite3_close(db);
        }
        break;
    case 4:
        printf("Модель: движение с поворотом.\n");
        h_m=MIN_HEIGHT;
        for(int i=0;i<time_sec;i++)
        {
            if(i>time_sec-5900 && i<time_sec-5650)
            {
                az_m2sec=1.06;
                if(vz_msec<UP_SPEED)
                {
                    vz_msec+=az_m2sec-g_m2Sec;
                }
                else
                {
                    az_m2sec=-g_m2Sec;
                    vz_msec=UP_SPEED;
                }
                
                if(h_m<MAX_HEIGHT)
                {
                    h_m+=vz_msec;    
                }
                else
                {
                    h_m=MAX_HEIGHT;
                }
            }
            else if (i>time_sec-5650 && i<time_sec-5648)
            {
                voy_csec=DOWN_ANGLE;                
            }
            else if(i>time_sec-5649 && i<time_sec-2154)
            {
                oy_c=DOWN_ANGLE;            
                voy_csec=0;
                ax_m2sec=0.64;
                Bx_G-=0.0000000256;
                Bz_G+=0.0000000256;
                if(vx_msec<MAX_SPEED)
                {
                    vx_msec+=ax_m2sec;
                }
                else
                {
                    ax_m2sec=0;
                    vx_msec=MAX_SPEED;
                }                
                if(h_m<MAX_HEIGHT)
                {
                    h_m+=vz_msec;   
                }
                else
                {
                    vz_msec=0;
                    h_m=MAX_HEIGHT;
                }
                X+=vx_msec;
            }
            else if (i>time_sec-2154 && i<time_sec-2152)
            {
                voy_csec=0;
                oy_c=voy_csec;
            }
            else if (i>time_sec-2152 && i<time_sec-2150)
            {
                voy_csec=UP_ANGLE;
            }
            else if (i>time_sec-2151 && i<time_sec-2103)
            {
                oy_c=UP_ANGLE;
                voy_csec=0;
                ax_m2sec=-0.64;
                if(vx_msec>35)
                {
                    vx_msec+=ax_m2sec;
                }
                else if(vx_msec<=35)
                {                
                    vx_msec=35;
                }
                if (vx_msec==35)
                {
                    ax_m2sec=0;
                }                
                X+=vx_msec;
            }
            else if (i>time_sec-2103 && i<time_sec-1101)
            {
                voy_csec=0;
            }
            else if (i>time_sec-1101 && i<time_sec-1099)
            {
                oy_c=0;
                voy_csec=0;
                vox_csec=50;
            }
            else if (i>time_sec-1100 && i<time_sec-1090)
            {
                ox_c=vox_csec;                                
            }
            else if (i>time_sec-1090 && i<time_sec-1088)
            {
                vox_csec=0;
                ox_c=vox_csec;
                char *pos = strstr(direction, "North");
                if (pos != NULL) {
                    memcpy(pos, "East", strlen("East"));
                }
            }
            else if (i>time_sec-1088 && i<time_sec-1086)
            {
                voy_csec=DOWN_ANGLE;
            }
            else
            {
                oy_c=DOWN_ANGLE;
                voy_csec=0;
                ax_m2sec=0.64;
                Bx_G+=0.0000000256;
                By_G+=0.0000000256;
                if(vx_msec<MAX_SPEED)
                {
                    vx_msec+=ax_m2sec;
                }
                else
                {
                    ax_m2sec=0;
                    vx_msec=MAX_SPEED;
                }               
                X+=vx_msec;
            }            
            sqlite3 *db;
            sqlite3_stmt *res;
            char *err_msg=0;
            int rc=sqlite3_open("Logs.db",&db);
            if(rc !=SQLITE_OK)
            {
                sqlite3_close(db);
                return 1;
            }   
            rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
            if(rc==SQLITE_OK)
            {
                sqlite3_bind_int(res,1,i);
                sqlite3_bind_double(res,2,X);
                sqlite3_bind_double(res,3,Y);
                sqlite3_bind_double(res,4,h_m);
                sqlite3_bind_text(res,5,direction,-1,SQLITE_STATIC);
                sqlite3_bind_double(res,6,ox_c);
                sqlite3_bind_double(res,7,oy_c);
                sqlite3_bind_double(res,8,oz_c);
                sqlite3_bind_double(res,9,vx_msec);
                sqlite3_bind_double(res,10,vy_msec);
                sqlite3_bind_double(res,11,vz_msec);
                sqlite3_bind_double(res,12,vox_csec);
                sqlite3_bind_double(res,13,voy_csec);
                sqlite3_bind_double(res,14,voz_csec);
                sqlite3_bind_double(res,15,ax_m2sec);
                sqlite3_bind_double(res,16,ay_m2sec);
                sqlite3_bind_double(res,17,az_m2sec);
                sqlite3_bind_double(res,18,Bx_G);
                sqlite3_bind_double(res,19,By_G);
                sqlite3_bind_double(res,20,Bz_G);
                while (rc=sqlite3_step(res)==SQLITE_ROW){}                
            }
            else
            {
                printf ("SQL error: %s\n",err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                return 1;
            }
            sqlite3_finalize(res);
            sqlite3_close(db);
            }            
        break;
    default:
        printf("Такой модели полета нет");
        break;
    }
    return 0;
}