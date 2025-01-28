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
            "CREATE TABLE Accelerometrs(Time int,X_axis_speed float,Y_axis_speed float,Z_axis_speed float);";
    char *bar="DROP TABLE IF EXISTS Barometr;"
            "CREATE TABLE Barometr(Time int,real_height float,real_height_mbar float,data_bar float);";
    char *gyro="DROP TABLE IF EXISTS Gyroscopes;"
            "CREATE TABLE Gyroscopes(Time int,Roll float,Pitch float,Yaw float);";
    char *mag="DROP TABLE IF EXISTS Magnetometer;"
            "CREATE TABLE Magnetometer(Time int,X float,Y float,Z float,B_horizontal float,Azimut float);";      
    char *model="DROP TABLE IF EXISTS model_flight;"
            "CREATE TABLE model_flight(Time int,Roll float,Pitch float,Yaw float,X_axis_acceleration float,Y_axis_acceleration float,Z_axis_acceleration float,X float,Y float,Z float,Height float);";
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
    sqlite3_close(db);
    return 0;
}
