#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

#define Time_work 100//время работы

int get_time()//передача времени
{
    return Time_work;
}

int flight(int num_model)
{
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
        for(int i=0;i<Time_work;i++)
        {
            h_m=150;
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
        for(int i=0;i<Time_work;i++)
        {
            h_m=150;
            vx_msec=13;
            oy_c=-30;
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
        printf("Модель взлета и движения.\n");
        h_m=10;
        for(int i=0;i<Time_work;i++)
        {
            if(i>Time_work-90 && h_m<150)
            {
                az_m2sec=1.4;
                if(vz_msec<4)
                {
                    vz_msec+=az_m2sec-1;
                }
                else
                {
                    az_m2sec=-1;
                    vz_msec=4;
                }
                h_m+=vz_msec;
                if(h_m>150)
                {
                    h_m=150;
                }
            }
            else if (i>Time_work-49 && i<Time_work-47)
            {
                vz_msec=0;
                voy_csec=-30;                
            }
            else if(i>Time_work-48)
            {
                oy_c=-30;
                voy_csec=0;
                ax_m2sec=0.448;
                if(vx_msec<13)
                {
                    vx_msec+=ax_m2sec;
                }
                else
                {
                    ax_m2sec=0;
                    vx_msec=13;
                }
            }
            // else if (i>Time_work-18 && i<Time_work-16)
            // {
            //     voy_csec=30;
            //     oy_c=0;
            // }
            // else if (i>Time_work-16 && i<Time_work-14)
            // {
            //     voy_csec=30;
            //     oy_c=30;
            // }
            // else if (i>Time_work-14 && i<Time_work-10)
            // {
            //     voy_csec=0;
            //     ax_m2sec=-1.857;
            //     if(vx_msec>0)
            //     {
            //         vx_msec+=ax_m2sec;
            //     }
            //     else
            //     {
            //         ax_m2sec=0;
            //         vx_msec=0;
            //     }
            // }
            // else if (i>Time_work-10 && i<Time_work-8)
            // {
            //     voy_csec=-30;
            //     oy_c=0;
            // // }
            // else if (i>Time_work-8)
            // {
            //     az_m2sec=-1.375;
            //     if(vz_msec<3)
            //     {
            //         vz_msec+=ax_m2sec+1;
            //     }
            //     else
            //     {
            //         az_m2sec=1;
            //         vz_msec=3;
            //     }
            //     if(h_m>0)
            //     {
            //         h_m+=vz_msec;
            //     }
            //     else
            //     {
            //         h_m=0;
            //     }
            // }
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
    default:
        printf("Такой модели полета нет");
        break;
    }
    return 0;
}