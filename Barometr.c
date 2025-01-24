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
#define SIGMA_bar 0.025 
#define LIMIT 3.0   

void send_array_bar(double *array, size_t size, const char *host)
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

void generate_normal_distribution_bar(double *values, int n)
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
        z0 = u1 * factor * SIGMA_bar;
        z1 = u2 * factor * SIGMA_bar;

        if (z0 < -LIMIT * SIGMA_bar) z0 = -LIMIT * SIGMA_bar;
        if (z0 > LIMIT * SIGMA_bar) z0 = LIMIT * SIGMA_bar;
        if (z1 < -LIMIT * SIGMA_bar) z1 = -LIMIT * SIGMA_bar;
        if (z1 > LIMIT * SIGMA_bar) z1 = LIMIT * SIGMA_bar;

        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

// double model_fly()
// {
//     double h=1000;
//     return h;
// }

double data_bar(double h,double sys_er, int time_request,int NUM_SAMPLES)
{        
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));
    double *Bar = (double *)malloc(NUM_SAMPLES * sizeof(double));
    if (values == NULL || Bar==NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_distribution_bar(values, NUM_SAMPLES);
    double real_h=h;
    double P;
    double p;
    const double P_0=1013.25;
    const int H=8400;
    double stepen=real_h/8400;
    double e=exp(-stepen);
    P=P_0*e;
    p=P;
    double data_request;
    P+=sys_er; 
    for(int i=0;i<NUM_SAMPLES;i++)
    {
        P+=values[i];
        Bar[i]=P;
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO Barometr VALUES (%d,%f,%f,%f)", time_request, real_h, p, P);
        rc=sqlite3_exec(db,sql,0,0,&err_msg);
        if(rc!=SQLITE_OK)
        {
            printf ("SQL error: %s\n",err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_close(db);
        data_request=P;
    }
    // size_t size=NUM_SAMPLES;
    // if(size==NUM_SAMPLES)
    // {
    //     send_array_bar(Bar,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    free(values);
    return data_request;
}