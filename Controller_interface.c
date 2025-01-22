#include <stdio.h>
#include <stdlib.h>
#include "Barometr.c"
#include "gyroscopes.c"
#include "accelerometrs.c"
#include "Magnetometer.c"

#define NUM_SAMPLES 10

int main(void)
{
    int time_request=3;
    double roll=0.0;
    double pitch=0.0;
    double yaw=0.0;
    double data_roll,data_pitch,data_yaw;
    data_gyro(roll,pitch,yaw,time_request,NUM_SAMPLES,&data_roll,&data_pitch,&data_yaw);
    printf("Данные с гироскопа:\nRoll: %f\nPitch: %f\nYaw: %f\n",data_roll,data_pitch,data_yaw);
    double Z_axis_acceleration=9.81;
    double Y_axis_acceleration=0.0;
    double X_axis_acceleration=0.0;
    double Y_axis,Z_axis,X_axis;
    data_accel(Y_axis_acceleration,X_axis_acceleration,Z_axis_acceleration,time_request,NUM_SAMPLES,&Y_axis,&X_axis,&Z_axis);
    printf("Данные с акселерометра:\nX_axis: %f\nY_axis: %f\nZ_axis: %f\n",X_axis,Y_axis,Z_axis);
    double x=0;
    double y=0;
    double z=0;
    double data_xmG,data_ymG,data_zmG;
    data_mag(x,y,z,time_request,NUM_SAMPLES,&data_xmG,&data_ymG,&data_zmG);
    printf("Данные с магнетометра:\nX_mG: %f\nY_mG: %f\nZ_mG: %f\n",data_xmG,data_ymG,data_zmG);
    double h=10;
    double sys_er;
    printf("Введите погрешность от -4.5 до 4.5: \n");
    scanf("%lf",&sys_er);
    double bar=data_bar(h,sys_er,time_request,NUM_SAMPLES);
    printf("Данные с бародатчика: %f\n",bar);
    return 0;
}