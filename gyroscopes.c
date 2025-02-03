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
#define SIGMA_gyro 0.135//значение сигма для генерации
#define LIMIT 3.0   //ограничения дипозона 
#define DT 1

void send_gyro(double *array, size_t size, const char *host)//отправка случайных значений для отрисовки графика по udp
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

void generate_normal_gyro(double *values, int n)//генерация случайных значений нормальным распределением
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
        z0 = u1 * factor * SIGMA_gyro;
        z1 = u2 * factor * SIGMA_gyro;
        if (z0 < -LIMIT * SIGMA_gyro) z0 = -LIMIT * SIGMA_gyro;
        if (z0 > LIMIT * SIGMA_gyro) z0 = LIMIT * SIGMA_gyro;
        if (z1 < -LIMIT * SIGMA_gyro) z1 = -LIMIT * SIGMA_gyro;
        if (z1 > LIMIT * SIGMA_gyro) z1 = LIMIT * SIGMA_gyro;
        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

void integrade_angle(double *x_Csek,double *y_Csek,double *z_Csek,double Dt)
{
    double angle=0;
    *x_Csek=angle+*x_Csek*Dt;
    *y_Csek=angle+*y_Csek*Dt;
    *z_Csek=angle+*z_Csek*Dt;
}

double data_gyro(double roll,double pitch,double yaw,int time_request,int NUM_SAMPLES,double *data_roll,double *data_pitch,double * data_yaw)//главная функция
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для сл значений
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_gyro(values, NUM_SAMPLES);
    double x_Csek=roll;
    double y_Csek=pitch;
    double z_Csek=yaw;
    integrade_angle(&x_Csek,&y_Csek,&z_Csek,DT);
    double Roll=x_Csek;
    double Pitch=y_Csek;     //для того чтобы основное число не менялось, а менялся только шум
    double Yaw=z_Csek;
    if(time_request==0)
    {
        *data_roll+=values[0];
        *data_pitch+=values[33];
        *data_yaw+=values[65];
    }
    else if(time_request==1)
    {
        *data_roll+=values[1];
        *data_pitch+=values[43];
        *data_yaw+=values[86];
    }
    else if (time_request==2)
    {
        *data_roll+=values[3];
        *data_pitch+=values[21];
        *data_yaw+=values[62];
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {        
            x_Csek+=values[i];
            y_Csek+=values[i-1];
            z_Csek+=values[i-2];
            *data_roll=x_Csek;
            *data_pitch=y_Csek;
            *data_yaw=z_Csek;        
            x_Csek=Roll;
            y_Csek=Pitch;
            z_Csek=Yaw;
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
    snprintf(sql, sizeof(sql), "INSERT INTO Gyroscopes VALUES (%d,%f,%f,%f)", time_request, *data_roll, *data_pitch, *data_yaw);//угол наклона
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
    //     send_gyro(values,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    free(values);
}