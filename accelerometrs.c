#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER_SIZE 1024 
#define SIGMA_accel 1.29
#define LIMIT 3.0   

void send_array_accel(double *array, size_t size, const char *host)
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

void generate_normal_distribution_accel(double *values, int n)
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

// double model_fly(double *Y_axis_acceleration, double *X_axis_acceleration,double *Z_axis_acceleration)
// {
//     *Y_axis_acceleration=0.0;//м/с^2 ось Y
//     *X_axis_acceleration=0.0;// ось X
//     *Z_axis_acceleration=9.81;//ось Z
// }

double data_accel(double Y_axis_acceleration,double X_axis_acceleration,double Z_axis_acceleration,int time_request,int NUM_SAMPLES,double *Y_axis,double *X_axis,double *Z_axis)
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));
    double *test = (double *)malloc(NUM_SAMPLES * sizeof(double));
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_distribution_accel(values, NUM_SAMPLES);
    double Z=Z_axis_acceleration;
    double Y=Y_axis_acceleration;
    double X=X_axis_acceleration;
    printf("Время:\tУскорение по оси X:\tУскорение по оси Y:\tУскорение по оси Z:\n");
    for(int i=0;i<NUM_SAMPLES;i++){
        Y_axis_acceleration+=values[i];
        X_axis_acceleration+=values[i];
        Z_axis_acceleration+=values[i];
        test[i]=Z_axis_acceleration;
        printf("  %d\t    %f\t            %f\t            %f\n",time_request,X_axis_acceleration,Y_axis_acceleration,Z_axis_acceleration);
        *Y_axis=Y_axis_acceleration;
        *X_axis=X_axis_acceleration;
        *Z_axis=Z_axis_acceleration;
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%d,%f,%f,%f)", time_request, X_axis_acceleration, Y_axis_acceleration, Z_axis_acceleration);
        rc=sqlite3_exec(db,sql,0,0,&err_msg);
        if(rc!=SQLITE_OK)
        {
            printf ("SQL error: %s\n",err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_close(db);
        Z_axis_acceleration=Z;
        Y_axis_acceleration=Y;
        X_axis_acceleration=X;
    }
    // char answer;
    // printf("Хотите отправить данные для построения графика(y/n))?");
    // scanf("%s",&answer);
    // if(answer=='y')
    // {
    // size_t size=NUM_SAMPLES;
    // if(size==NUM_SAMPLES)
    // {
    //     send_array_accel(test,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    // }
    // else
    // {
    //     exit;
    // }
    free(values);  
}