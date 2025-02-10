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

double vx,vy,vz;

double data_accel(double *aX_m2sec,double *aY_m2sec,double *aZ_m2sec,double vx_msec,double vy_msec,double vz_msec,int num,int time_request,int count,double *Y_axis_msec,double *X_axis_msec,double *Z_axis_msec)//главная функция
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
        if(*aX_m2sec!=0 || *aY_m2sec!=0 || *aZ_m2sec!=1 || *aZ_m2sec!=-1)
        {        
        double t_start_x=time_request-1,t_end_x=time_request;
        double t_start_y=time_request-1,t_end_y=time_request;
        double t_start_z=time_request-1,t_end_z=time_request;
        X_msec=integrate(*aX_m2sec,t_start_x,t_end_x);
        Y_msec=integrate(*aY_m2sec,t_start_y,t_end_y);//преобразование в скорость
        Z_msec=integrate(*aZ_m2sec,t_start_z,t_end_z);
        vy+=Y_msec;
        if(X_msec<13)
        {
            vx+=X_msec;
        }
        else
        {
            vx=13;
        }
        if(Z_msec<4 && Z_msec>0)
        {
            vz+=Z_msec-1;
        }
        else if(time_request<53)
        {
            vz=4;
        }
        else
        {
            vz=0;
        }
        }
        else
        {
            X_msec=vx;
            Y_msec=vy;
            Z_msec=vz;
        }

    }
    else if (num==2)
    {
        X_msec=vx_msec;
        Y_msec=vy_msec;
        Z_msec=vz_msec;
    }
    double x_msec=vx;
    double y_msec=vy;//для того чтобы основное число не менялось, а менялся только шум
    double z_msec=vz;
    double ax=*aX_m2sec;
    double ay=*aY_m2sec;
    double az=*aZ_m2sec;
    double aX=ax,aY=ay,aZ=az;

   if(time_request==0)
    {
        X_msec+=values[0];
        Y_msec+=values[78];
        Z_msec+=values[22];
        *aX_m2sec+=values[47];
        *aY_m2sec+=values[4];
        *aZ_m2sec+=values[67];
        *X_axis_msec=X_msec;
        *Y_axis_msec=Y_msec;
        *Z_axis_msec=Z_msec; 
        X_msec=x_msec;
        Y_msec=y_msec;
        Z_msec=z_msec;
    }
    else if(time_request ==1)
    {
        X_msec+=values[1];
        Y_msec+=values[44];
        Z_msec+=values[43];
        *aX_m2sec+=values[23];
        *aY_m2sec+=values[5];
        *aZ_m2sec+=values[65];
        *X_axis_msec=X_msec;
        *Y_axis_msec=Y_msec;
        *Z_axis_msec=Z_msec; 
        X_msec=x_msec;
        Y_msec=y_msec;
        Z_msec=z_msec;
    }
    else if(time_request ==2)
    {
        X_msec+=values[2];
        Y_msec+=values[87];
        Z_msec+=values[91];
        *aX_m2sec+=values[85];
        *aY_m2sec+=values[6];
        *aZ_m2sec+=values[12];
        *X_axis_msec=X_msec;
        *Y_axis_msec=Y_msec;
        *Z_axis_msec=Z_msec; 
        X_msec=x_msec;
        Y_msec=y_msec;
        Z_msec=z_msec;
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {   
            X_msec+=values[i];     
            Y_msec+=values[i-1];
            Z_msec+=values[i-2];
            ax+=values[i];
            ay+=values[i];
            az+=values[i];
            // test[i]=Z_axis_acceleration;    
            *aX_m2sec=ax;
            *aY_m2sec=ay;
            *aZ_m2sec=az;
            *X_axis_msec=X_msec;
            *Y_axis_msec=Y_msec;
            *Z_axis_msec=Z_msec;        
            X_msec=x_msec;
            Y_msec=y_msec;
            Z_msec=z_msec;
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