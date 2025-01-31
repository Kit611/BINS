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
            "CREATE TABLE Accelerometrs(Time_sek int,X_axis_speed_msek float,Y_axis_speed_msek float,Z_axis_speed_msek float);";
    char *bar="DROP TABLE IF EXISTS Barometr;"
            "CREATE TABLE Barometr(Time_sek int,real_height_m float,real_height_mbar float,data_bar_mbar float);";
    char *gyro="DROP TABLE IF EXISTS Gyroscopes;"
            "CREATE TABLE Gyroscopes(Time_sek int,Roll_grad float,Pitch_grad float,Yaw_grad float);";
    char *mag="DROP TABLE IF EXISTS Magnetometer;"
            "CREATE TABLE Magnetometer(Time_sek int,X_mG float,Y_mG float,Z_mG float,Declination_grad float,Inclination_grad float,True_direction_grad float);";      
    char *model="DROP TABLE IF EXISTS model_flight;"
            "CREATE TABLE model_flight(Time_sek int,Roll_gradsek float,Pitch_gradsek float,Yaw_gradsek float,X_axis_acceleration_m2sek float,Y_axis_acceleration_m2Sek float,Z_axis_acceleration_m2sek float,X_nT float,Y_nT float,Z_nT float,Height_m float);";
    char *finish="DROP TABLE IF EXISTS model_flight;"
            "CREATE TABLE finish_data(Time_sek int,Roll_gradsek float,Pitch_gradsek float,Yaw_gradsek float,X_speed_msek float,Y_speed_msek float,Z_speed_msek float,x_mG float,y_mG float,z_mG float,Height_mbar float);";
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
