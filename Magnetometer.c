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
#define SIGMA_mag 0.22//значение сигма для генерации
#define LIMIT 3.0   //ограничение диапозона

void send_mag(double *array, size_t size, const char *host)//отправка случайных значений для отрисовки графика по udp
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

void generate_normal_mag(double *values, int n)//генерация случайных значений нормальным распределением
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
        z0 = u1 * factor * SIGMA_mag;
        z1 = u2 * factor * SIGMA_mag;
        if (z0 < -LIMIT * SIGMA_mag) z0 = -LIMIT * SIGMA_mag;
        if (z0 > LIMIT * SIGMA_mag) z0 = LIMIT * SIGMA_mag;
        if (z1 < -LIMIT * SIGMA_mag) z1 = -LIMIT * SIGMA_mag;
        if (z1 > LIMIT * SIGMA_mag) z1 = LIMIT * SIGMA_mag;
        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

double Course(double bx,double by,double *B_hor,double *azimut)//определение азимута и направление в горизонтальной плоскости
{
    *B_hor=sqrt(pow(bx,2)+pow(by,2));
    *azimut=atan2(by,bx);
}

// float model_fly(double *x,double *y,double *z)//не нужное
// {
//     *x=0;
//     *y=0; //gauss
//     *z=0;
// }

double data_mag(double x,double y,double z,int time_request,int NUM_SAMPLES,double lon,double lat,double *data_x,double *data_y,double *data_z)//главная фукнция
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для сл значений
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }    
    generate_normal_mag(values, NUM_SAMPLES);
    double B_hor,azimut;//горизонтальный курс и азимут
    double X=x;
    double Y=y;//для того чтобы основное число не менялось, а менялся только шум
    double Z=z;
    if(time_request==0)
    {
        *data_x=values[0];
        *data_y=values[45];
        *data_z=values[56];
    }
    else if(time_request ==1)
    {
        *data_x=values[1];
        *data_y=values[12];
        *data_z=values[76];
    }
    else if(time_request ==2)
    {
        *data_x=values[2];
        *data_y=values[48];
        *data_z=values[98];
    }
    else
    {
        for(int i=0;i<time_request;i++)
        {        
            x+=values[i];
            y+=values[i-1];
            z+=values[i-2];
            *data_x=x;
            *data_y=y;
            *data_z=z;        
            x=X;
            y=Y;
            z=Z;
        }
    }
    Course(*data_x,*data_y,&B_hor,&azimut);
    sqlite3 *db;//запись в бд
    char *err_msg=0;
    int rc=sqlite3_open("Logs.db",&db);
    if(rc !=SQLITE_OK)
    {
    sqlite3_close(db);
    return 1;
    }   
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Magnetometer VALUES (%d,%f,%f,%f,%f,%f)", time_request, *data_x, *data_y, *data_z,B_hor,azimut);//значения магнетометра с шумом и выше
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
    //     send_mag(values,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    free(values);
}