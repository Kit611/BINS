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
#define SIGMA_mag 0.22
#define LIMIT 3.0  

void send_array_mag(double *array, size_t size, const char *host)
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

void generate_normal_distribution_mag(double *values, int n)
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

// float model_fly(double *x,double *y,double *z)
// {
//     *x=0;
//     *y=0; //gauss
//     *z=0;
// }

double data_mag(double x,double y,double z,int time_request,int NUM_SAMPLES,double *data_xmG,double *data_ymG,double *data_zmG)
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_distribution_mag(values, NUM_SAMPLES);
    double x_mG=x*1000;
    double y_mG=y*1000; //в милигаусс
    double z_mG=z*1000;
    printf("Время:\tМагнитное поле по оси X:\tМагнитное поле по оси Y:\tМагнитное поле по оси Z:\n");
    double X=x_mG;
    double Y=y_mG;
    double Z=z_mG;
    for(int i=0;i<NUM_SAMPLES;i++)
    {
        x_mG+=values[i];
        y_mG+=values[i];
        z_mG+=values[i];
        printf("  %d\t          %f\t                 %f\t                  %f\n",i,x_mG,y_mG,z_mG);
        if(i==time_request)
        {
            *data_xmG=x_mG;
            *data_ymG=y_mG;
            *data_zmG=z_mG;
        }
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO Magnetometer VALUES (%d,%f,%f,%f)", i, x_mG, y_mG, z_mG);
        rc=sqlite3_exec(db,sql,0,0,&err_msg);
        if(rc!=SQLITE_OK)
        {
            printf ("SQL error: %s\n",err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_close(db);
        x_mG=X;
        y_mG=Y;
        z_mG=Z;
    }
    char answer;
    printf("Хотите отправить данные для построения графика(y/n))?");
    scanf("%s",&answer);
    if(answer=='y')
    {
    size_t size=NUM_SAMPLES;
    if(size==NUM_SAMPLES)
    {
        send_array_mag(values,size,SERVER_IP);
        printf("%s","Отправлено\n");
    }
    else
    {
        printf("Количество элементов: %ld", size);
    }
    }
    else
    {
        exit;
    }
    free(values);
}