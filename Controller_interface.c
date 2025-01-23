#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "Barometr.c"
#include "gyroscopes.c"
#include "accelerometrs.c"
#include "Magnetometer.c"

#define NUM_SAMPLES 1

int data_bd(int time_request,int *time,double *roll,double *pitch,double *yaw,double *X_axis_acceleration,double * Y_axis_acceleration,double *Z_axis_acceleration,double *x,double *y,double *z,double *h)
{
sqlite3 *db;
    sqlite3_stmt *res;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc!=SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char *sql="SELECT * FROM model_fly WHERE Time=?";
    rc =sqlite3_prepare_v2(db,sql,-1,&res,0);
    if(rc==SQLITE_OK)
    {
        sqlite3_bind_int(res,1,time_request);
        while(sqlite3_step(res)==SQLITE_ROW){
            *time=sqlite3_column_int(res,0);
            *roll=sqlite3_column_double(res,1);
            *pitch=sqlite3_column_double(res,2);
            *yaw=sqlite3_column_double(res,3);
            *X_axis_acceleration=sqlite3_column_double(res,4);
            *Y_axis_acceleration=sqlite3_column_double(res,5);
            *Z_axis_acceleration=sqlite3_column_double(res,6);
            *x=sqlite3_column_double(res,7);
            *y=sqlite3_column_double(res,8);
            *z=sqlite3_column_double(res,9);
            *h=sqlite3_column_double(res,10);
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

int main(void)
{
    int time_request[2]={2,4};
    int time;
    double roll,pitch,yaw;
    double Z_axis_acceleration,Y_axis_acceleration,X_axis_acceleration;
    double x,y,z;
    double h;
    double sys_er;
    int size=sizeof(time_request)/sizeof(time_request[0]);
    printf("Введите погрешность от -4.5 до 4.5: \n");
    scanf("%lf",&sys_er);
    for(int i=0;i<size;i++)
    {
    data_bd(time_request[i],&time,&roll,&pitch,&yaw,&X_axis_acceleration,&Y_axis_acceleration,&Z_axis_acceleration,&x,&y,&z,&h);
    double data_roll,data_pitch,data_yaw;
    data_gyro(roll,pitch,yaw,time_request[i],NUM_SAMPLES,&data_roll,&data_pitch,&data_yaw);
    // printf("Данные с гироскопа:\nRoll: %f\nPitch: %f\nYaw: %f\n",data_roll,data_pitch,data_yaw);
    double Y_axis,Z_axis,X_axis;
    data_accel(Y_axis_acceleration,X_axis_acceleration,Z_axis_acceleration,time_request[i],NUM_SAMPLES,&Y_axis,&X_axis,&Z_axis);
    // printf("Данные с акселерометра:\nX_axis: %f\nY_axis: %f\nZ_axis: %f\n",X_axis,Y_axis,Z_axis);
    double data_xmG,data_ymG,data_zmG;
    data_mag(x,y,z,time_request[i],NUM_SAMPLES,&data_xmG,&data_ymG,&data_zmG);
    // printf("Данные с магнетометра:\nX_mG: %f\nY_mG: %f\nZ_mG: %f\n",data_xmG,data_ymG,data_zmG);
    double bar=data_bar(h,sys_er,time_request[i],NUM_SAMPLES);
    }
    // printf("Данные с бародатчика: %f\n",bar);
    return 0;
}