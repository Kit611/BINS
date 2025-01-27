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
#define SIGMA_bar 0.025 //значение сигма для генерации
#define LIMIT 3.0   //ограничение диапозона

void send_bar(double *array, size_t size, const char *host)//отправка случайных значений для отрисовки графика по udp
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

void generate_normal_bar(double *values, int n)//генерация случайных значений нормальным распределением
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

// double model_fly()//не нужное
// {
//     double h=1000;
//     return h;
// }

double data_bar(double h,double sys_er, int time_request,int NUM_SAMPLES)//главная функция
{        
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для сл значений
    // double *Bar = (double *)malloc(NUM_SAMPLES * sizeof(double));//массив для отправки итоговых значений для графика
    if (values == NULL ) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_bar(values, NUM_SAMPLES);
    double real_h=h;
    double P;
    double p;
    const double P_0=1013.25;//давление на станадартной высоте в милибарах
    const int H=8400;//стандартная высота
    double stepen=real_h/8400;
    double e=exp(-stepen);
    P=P_0*e;//перевод в милибары
    p=P;
    double data_request;
    P+=sys_er; //добавление системной ошибки
    for(int i=0;i<time_request;i++)
    {
        P+=values[i];
        data_request=P; 
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
    snprintf(sql, sizeof(sql), "INSERT INTO Barometr VALUES (%d,%f,%f,%f);", time_request, real_h, p, P);//давление в милибарах с шумом
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
    //     send_bar(values,size,SERVER_IP);
    //     printf("%s","Отправлено\n");
    // }
    // else
    // {
    //     printf("Количество элементов: %ld", size);
    // }
    free(values);
    return data_request;
}