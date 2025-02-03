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

double integrate(double acceleration, double t_start, double t_end) {//функция интегрирования дял получения скорости
    double dt = t_end - t_start;
    double initial_velocity = 0;
    double final_velocity = initial_velocity + acceleration * dt;

    return final_velocity;
}

double data_accel(double Y_axis_acceleration,double X_axis_acceleration,double Z_axis_acceleration,int time_request,int NUM_SAMPLES,double *Y_axis,double *X_axis,double *Z_axis)//главная функция
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для сл значений
    double t_start_y=14,t_end_y=71;
    double t_start_x=0,t_end_x=1;
    double t_start_z=4,t_end_z=13;
    //double *test = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для отправки итоговых значений для графика
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_accel(values, NUM_SAMPLES);
    double Z=integrate(Z_axis_acceleration,t_start_z,t_end_z);
    double Y=integrate(Y_axis_acceleration,t_start_y,t_end_y);//преобразование в скорость
    double X=integrate(X_axis_acceleration,t_start_x,t_end_x);
    double x=X;
    double y=Y;//для того чтобы основное число не менялось, а менялся только шум
    double z=Z;
    if(time_request==0)
    {
        *X_axis=values[0];
        *Y_axis=values[78];
        *Z_axis=values[22];
    }
    else if(time_request ==1)
    {
        *X_axis=values[1];
        *Y_axis=values[44];
        *Z_axis=values[43];
    }
    else if(time_request ==2)
    {
        *X_axis=values[2];
        *Y_axis=values[87];
        *Z_axis=values[91];
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {        
            Y+=values[i];
            X+=values[i-1];
            Z+=values[i-2];
            // test[i]=Z_axis_acceleration;    
            *Y_axis=Y;
            *X_axis=X;
            *Z_axis=Z;        
            Z=z;
            Y=y;
            X=x;
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
    snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%d,%f,%f,%f)", time_request, *X_axis, *Y_axis, *Z_axis);//скорось по трем осям
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