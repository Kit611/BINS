#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SERVER_PORT 12345//порт
#define SERVER_IP "127.0.0.1"//ip
#define MAX_BUFFER_SIZE 1024 //максимальное значение
#define SIGMA_accel 1.29//значение сигма для генерации
#define LIMIT 3.0   //ограничения дипозона 

void send_accel(double *array, size_t size, const char *host)//отправка случайных значений для отрисовки графика по udp
{
    int sock;
    struct sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, host, &server_addr.sin_addr);
    for (size_t i = 0; i < size; i++)
    {
        char buffer[MAX_BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "%f", array[i]);
        ssize_t sent_bytes = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0) 
        {
            perror("Ошибка отправки данных");
        }
        usleep(1000);
    }
    close(sock);
}

void generate_normal_accel(double *values, int n)//генерация случайных значений нормальным распределением
{
    int i = 0;
    while (i < n)
    {
        double u1, u2, s, z0, z1;
        do
        {
            u1 = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            u2 = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            s = u1 * u1 + u2 * u2;
        } while (s >= 1 || s == 0);

        double factor = sqrt(-2.0 * log(s) / s);
        z0 = u1 * factor * SIGMA_accel;
        z1 = u2 * factor * SIGMA_accel;
        if (z0 < -LIMIT * SIGMA_accel) z0 = -LIMIT * SIGMA_accel;
        if (z0 > LIMIT * SIGMA_accel) z0 = LIMIT * SIGMA_accel;
        if (z1 < -LIMIT * SIGMA_accel) z1 = -LIMIT * SIGMA_accel;
        if (z1 > LIMIT * SIGMA_accel) z1 = LIMIT * SIGMA_accel;
        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

double integrate(double acceleration, double t_start, double t_end)//функция интегрирования для получения скорости
{
    double dt = t_end - t_start;
    double initial_velocity = 0;
    double final_velocity = initial_velocity + acceleration * dt;
    return final_velocity;
}

double data_accel(double *aY_m2sec,double *aX_m2sec,double *aZ_m2sec,double vx_msec,double vy_msec,double vz_msec,int num,int time_request,int count,double *Y_axis_msec,double *X_axis_msec,double *Z_axis_msec)//главная функция
{
    srand(time(NULL));
    double *values = (double *)malloc(count * sizeof(double));//массив для сл значений
    //double *test = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для отправки итоговых значений для графика
    double X_msec,Y_msec,Z_msec;
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_accel(values, count);
    if(num==3)
    {
        double t_start_y=14,t_end_y=71;
        double t_start_x=0,t_end_x=1;
        double t_start_z=4,t_end_z=13;
        Z_msec=integrate(*aZ_m2sec,t_start_z,t_end_z);
        Y_msec=integrate(*aY_m2sec,t_start_y,t_end_y);//преобразование в скорость
        X_msec=integrate(*aX_m2sec,t_start_x,t_end_x);
    }
    else if (num==2)
    {
        X_msec=vx_msec;
        Y_msec=vy_msec;
        Z_msec=vz_msec;
    }
    double x_msec=X_msec;
    double y_msec=Y_msec;//для того чтобы основное число не менялось, а менялся только шум
    double z_msec=Z_msec;
    double ax=*aX_m2sec;
    double ay=*aY_m2sec;
    double az=*aZ_m2sec;
    double aX=ax,aY=ay,aZ=az;
    if(time_request==0)
    {
        X_msec+=values[0];
        Y_msec+=values[78];
        Z_msec+=values[22];
        *aY_m2sec+=values[4];
        *aX_m2sec+=values[47];
        *aZ_m2sec+=values[67];
        *Y_axis_msec=Y_msec;
        *X_axis_msec=X_msec;
        *Z_axis_msec=Z_msec; 
        Z_msec=z_msec;
        Y_msec=y_msec;
        X_msec=x_msec;
    }
    else if(time_request ==1)
    {
        X_msec+=values[1];
        Y_msec+=values[44];
        Z_msec+=values[43];
        *aY_m2sec+=values[5];
        *aX_m2sec+=values[23];
        *aZ_m2sec+=values[65];
        *Y_axis_msec=Y_msec;
        *X_axis_msec=X_msec;
        *Z_axis_msec=Z_msec; 
        Z_msec=z_msec;
        Y_msec=y_msec;
        X_msec=x_msec;
    }
    else if(time_request ==2)
    {
        X_msec+=values[2];
        Y_msec+=values[87];
        Z_msec+=values[91];
        *aY_m2sec+=values[6];
        *aX_m2sec+=values[85];
        *aZ_m2sec+=values[12];
        *Y_axis_msec=Y_msec;
        *X_axis_msec=X_msec;
        *Z_axis_msec=Z_msec; 
        Z_msec=z_msec;
        Y_msec=y_msec;
        X_msec=x_msec;
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {        
            Y_msec+=values[i];
            X_msec+=values[i-1];
            Z_msec+=values[i-2];
            ax+=values[i];
            ay+=values[i];
            az+=values[i];
            // test[i]=Z_axis_acceleration;    
            *aY_m2sec=ay;
            *aX_m2sec=ax;
            *aZ_m2sec=az;
            *Y_axis_msec=Y_msec;
            *X_axis_msec=X_msec;
            *Z_axis_msec=Z_msec;        
            Z_msec=z_msec;
            Y_msec=y_msec;
            X_msec=x_msec;
            ax=aX;
            ay=aY;
            az=aZ;
        }
    }
    sqlite3 *db;//запись в бд
    char *err_msg=0;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc !=SQLITE_OK)
    {
    sqlite3_close(db);
    return 1;
    }   
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%d,%f,%f,%f)", time_request, *X_axis_msec, *Y_axis_msec, *Z_axis_msec);//скорось по трем осям
    rc=sqlite3_exec(db,sql,0,0,&err_msg);
    if(rc!=SQLITE_OK)
    {
        printf ("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    // size_t size=NUM_SAMPLES;
    // if(size==NUM_SAMPLES)
    // {
    //     send_accel(values,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    free(values);  
}