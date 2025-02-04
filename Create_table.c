#include <stdio.h>
#include <sqlite3.h>

int main(){
    sqlite3 *db;
    char *err_msg=0;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc !=SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char *accel="DROP TABLE IF EXISTS Accelerometrs;"
            "CREATE TABLE Accelerometrs(Time_sec int,X_axis_speed_msec float,Y_axis_speed_msec float,Z_axis_speed_msec float);";
    char *bar="DROP TABLE IF EXISTS Barometr;"
            "CREATE TABLE Barometr(Time_sec int,real_height_m float,real_height_mbar float,data_bar_mbar float);";
    char *gyro="DROP TABLE IF EXISTS Gyroscopes;"
            "CREATE TABLE Gyroscopes(Time_sec int,Roll_grad float,Pitch_grad float,Yaw_grad float);";
    char *mag="DROP TABLE IF EXISTS Magnetometer;"
            "CREATE TABLE Magnetometer(Time_sec int,X_mG float,Y_mG float,Z_mG float,Declination_grad float,Inclination_grad float,True_direction_grad float);";      
    char *model="DROP TABLE IF EXISTS model_flight;"
            "CREATE TABLE model_flight(Time_sec int,X float,Y float, H_m float,Ox_C float,Oy_C float,Oz_C float,Vx_mSec float, Vy_mSec float,Vz_mSec float,Vox_cSec float,Voy_cSec float,Voz_cSec float,ax_m2Sec float,ay_m2Sec float,az_m2Sec float, declination_c float,inclination_c float);";
    char *finish="DROP TABLE IF EXISTS finish_data;"
            "CREATE TABLE finish_data(Time_sec int,X float,Y float, H_m float,Ox_C float,Oy_C float,Oz_C float,Vx_mSec float, Vy_mSec float,Vz_mSec float,Vox_cSec loat,Voy_Csec float,Voz_Csec float,ax_m2Sec float,ay_m2Sec float,az_m2Sec float, declination_c float,inclination_c float);";
    rc=sqlite3_exec(db,accel,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    rc=sqlite3_exec(db,bar,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    rc=sqlite3_exec(db,gyro,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    rc=sqlite3_exec(db,mag,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    rc=sqlite3_exec(db,model,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    rc=sqlite3_exec(db,finish,0,0,&err_msg);
    if(rc !=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    return 0;
}
