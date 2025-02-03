#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "Barometr.c"
#include "gyroscopes.c"
#include "accelerometrs.c"
#include "Magnetometer.c"
#include "model_flight.c"

#define NUM_SAMPLES 1

int data_bd(int time_request,int *time,double *lon,double *lat,double *roll_C_sec,double *pitch_C_sec,double *yaw_C_sec,double *X_axis_acceleration_m2_sec,double * Y_axis_acceleration_m2_sec,double *Z_axis_acceleration_m2_sec,double *x_nT,double *y_nT,double *z_nT,double *h_m)//получение данных из бд
{
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc!=SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char *sql="SELECT * FROM model_flight WHERE Time_sec=?";
    rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
    if(rc==SQLITE_OK)
    {
        sqlite3_bind_int(res,1,time_request);
        while(sqlite3_step(res)==SQLITE_ROW)
        {
            *time=sqlite3_column_int(res,0);            
            *roll_C_sec=sqlite3_column_double(res,1);
            *pitch_C_sec=sqlite3_column_double(res,2);
            *yaw_C_sec=sqlite3_column_double(res,3);
            *X_axis_acceleration_m2_sec=sqlite3_column_double(res,4);
            *Y_axis_acceleration_m2_sec=sqlite3_column_double(res,5);
            *Z_axis_acceleration_m2_sec=sqlite3_column_double(res,6);
            *x_nT=sqlite3_column_double(res,7);
            *y_nT=sqlite3_column_double(res,8);
            *z_nT=sqlite3_column_double(res,9);
            *h_m=sqlite3_column_double(res,10);
        }
    }
    else
    {
        fprintf(stderr,"Error: %s\n",sqlite3_errmsg(db));
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}

int write_bd(int time,double roll_grad,double pitch_grad,double yaw_grad,double X_axis_m_sec,double Y_axis_m_sec,double Z_axis_m_sec,double x_mG,double y_mG,double z_mG,double h_mbar)
{
    sqlite3 *db;
    char *err_msg=0;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc!=SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO finish_data VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",time,roll_grad,pitch_grad,yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,x_mG, y_mG,z_mG,h_mbar);
    rc=sqlite3_exec(db,sql,0,0,&err_msg);
    if(rc!=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    return 0;
}

int main(void)
{    
    double lon,lat;
    flight(&lon,&lat);//вызов модели полета
    int time_request=0;//время запроса
    int time_sec;//время которое возвращается
    double roll_C_sec,pitch_C_sec,yaw_C_sec;
    double Z_axis_acceleration_m2_sec,Y_axis_acceleration_m2_sec,X_axis_acceleration_m2_sec;
    double x_nT,y_nT,z_nT;
    double h_m;
    double sys_er;
    int work_time=get_time();//время работы
    printf("Введите погрешность от -4.5 до 4.5: ");
    scanf("%lf",&sys_er);//ввод системной ошибки
    printf("%-10s| %-42s | %-43s | %-40s | %s\n","Время(сек):","Гироскоп(град):","Акселерометр(g):","Магнетометер(mG):","Барометр(mbar):");
    for(int i=0;i<work_time;i++)
    {
        data_bd(time_request,&time_sec,&lon,&lat,&roll_C_sec,&pitch_C_sec,&yaw_C_sec,&X_axis_acceleration_m2_sec,&Y_axis_acceleration_m2_sec,&Z_axis_acceleration_m2_sec,&x_nT,&y_nT,&z_nT,&h_m);
        double data_roll_grad,data_pitch_grad,data_yaw_grad;
        data_gyro(roll_C_sec,pitch_C_sec,yaw_C_sec,time_request,work_time,&data_roll_grad,&data_pitch_grad,&data_yaw_grad);
        double Y_axis_m_sec,Z_axis_m_sec,X_axis_m_sec;
        data_accel(Y_axis_acceleration_m2_sec,X_axis_acceleration_m2_sec,Z_axis_acceleration_m2_sec,time_request,work_time,&Y_axis_m_sec,&X_axis_m_sec,&Z_axis_m_sec);
        double data_x_mG,data_y_mG,data_z_mG;
        data_mag(x_nT,y_nT,z_nT,time_request,work_time,lon,lat,&data_x_mG,&data_y_mG,&data_z_mG);
        double bar_mbar=data_bar(h_m,sys_er,time_request,work_time);
        printf("%-10d | (%f;%f;%f) | (%f;%f;%f) | (%f;%f;%f) | %f\n",i,data_roll_grad,data_pitch_grad,data_yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,data_x_mG,data_y_mG,data_z_mG,bar_mbar);
        time_request++;
        write_bd(i,data_roll_grad,data_pitch_grad,data_yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,data_x_mG,data_y_mG,data_z_mG,bar_mbar);
    }    
    return 0;
}