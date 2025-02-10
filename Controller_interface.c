#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "Barometr.c"
#include "gyroscopes.c"
#include "accelerometrs.c"
#include "Magnetometer.c"
#include "model_flight.c"

//получение из бд
int data_bd(int time_request,int *time,double *X,double *Y,double *h_m,double *ox_c,double *oy_c,double *oz_c,double *vx_msec,double *vy_msec,double *vz_msec,double *vox_csec,double *voy_csec,double *voz_csec,double *ax_m2sec,double *ay_m2sec,double *az_m2sec,double *Bx_G,double *By_G,double *Bz_G)//получение данных из бд
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
            *X=sqlite3_column_double(res,1);
            *Y=sqlite3_column_double(res,2);
            *h_m=sqlite3_column_double(res,3);
            *ox_c=sqlite3_column_double(res,4);
            *oy_c=sqlite3_column_double(res,5);
            *oz_c=sqlite3_column_double(res,6);
            *vx_msec=sqlite3_column_double(res,7);
            *vy_msec=sqlite3_column_double(res,8);
            *vz_msec=sqlite3_column_double(res,9);
            *vox_csec=sqlite3_column_double(res,10);
            *voy_csec=sqlite3_column_double(res,11);
            *voz_csec=sqlite3_column_double(res,12);
            *ax_m2sec=sqlite3_column_double(res,13);
            *ay_m2sec=sqlite3_column_double(res,14);
            *az_m2sec=sqlite3_column_double(res,15);
            *Bx_G=sqlite3_column_double(res,16);
            *By_G=sqlite3_column_double(res,17);
            *Bz_G=sqlite3_column_double(res,18);
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
//запись в бд
int write_bd(int time,double X,double Y,double h_mbar,double roll_grad,double pitch_grad,double yaw_grad,double X_axis_m_sec,double Y_axis_m_sec,double Z_axis_m_sec,double vox_csec,double voy_csec,double voz_csec,double ax_m2sec,double ay_m2sec,double az_m2sec,double x_mG,double y_mG,double z_mG,double declination_c,double inclination_c)
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
    snprintf(sql, sizeof(sql), "INSERT INTO finish_data VALUES (%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",time,X,Y,h_mbar,roll_grad,pitch_grad,yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,x_mG, y_mG,z_mG,declination_c,inclination_c);
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
    int num_model;
    printf("Введите модель полета(1-Висение;2-Линейный полет;3-взлет и движение): ");
    scanf("%d",&num_model);//выбор модели полета
    flight(num_model);//вызов модели полета
    int time_request=0;//время запроса
    int time_sec;//время которое возвращается
    double X,Y,h_m,ox_c,oy_c,oz_c,vx_msec,vy_msec,vz_msec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,Bx_G,By_G,Bz_G,declination_c=0,inclination_c=0;
    double sys_er;
    int work_time=get_time();//время работы
    printf("Введите погрешность от -4.5 до 4.5: ");
    scanf("%lf",&sys_er);//ввод системной ошибки
    printf("%-10s| %-42s | %-43s | %-40s | %s\n","Время(сек):","Гироскоп(град):","Акселерометр(g):","Магнетометер(mG):","Барометр(mbar):");
    for(int i=0;i<work_time;i++)
    {
        data_bd(time_request,&time_sec,&X,&Y,&h_m,&ox_c,&oy_c,&oz_c,&vx_msec,&vy_msec,&vz_msec,&vox_csec,&voy_csec,&voz_csec,&ax_m2sec,&ay_m2sec,&az_m2sec,&Bx_G,&By_G,&Bz_G);
        double data_roll_grad,data_pitch_grad,data_yaw_grad;
        double Y_axis_m_sec,Z_axis_m_sec,X_axis_m_sec;
        double data_x_mG,data_y_mG,data_z_mG;
        data_gyro(&vox_csec,&voy_csec,&voz_csec,ox_c,oy_c,oz_c,num_model,time_request,work_time,&data_roll_grad,&data_pitch_grad,&data_yaw_grad);//данные с гироскопа
        data_accel(&ax_m2sec,&ay_m2sec,&az_m2sec,vx_msec,vy_msec,vz_msec,num_model,time_request,work_time,&Y_axis_m_sec,&X_axis_m_sec,&Z_axis_m_sec);//данные с акселерометра
        data_mag(Bx_G,By_G,Bz_G,data_roll_grad,data_pitch_grad,data_yaw_grad,time_request,work_time,&data_x_mG,&data_y_mG,&data_z_mG,&declination_c,&inclination_c);//данные с магнетометра
        double bar_mbar=data_bar(h_m,sys_er,time_request,work_time);//данные с барометра
        printf("%-10d | (%f;%f;%f) | (%f;%f;%f) | (%f;%f;%f) | %f\n",i,data_roll_grad,data_pitch_grad,data_yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,data_x_mG,data_y_mG,data_z_mG,bar_mbar);
        time_request++;
        write_bd(i,X,Y,bar_mbar,data_roll_grad,data_pitch_grad,data_yaw_grad,X_axis_m_sec,Y_axis_m_sec,Z_axis_m_sec,vox_csec,voy_csec,voz_csec,ax_m2sec,ay_m2sec,az_m2sec,data_x_mG,data_y_mG,data_z_mG,declination_c,inclination_c);
    }    
    return 0;
}