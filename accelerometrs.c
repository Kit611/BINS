#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>

#define A 6378245.0          // Большая (экваториальная) полуось
#define B 6356863.019        // Малая (полярная) полуось
#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER_SIZE 1024 
#define NUM_SAMPLES 10
#define SIGMA 1.29
#define LIMIT 3.0   

void send_array(double *array, size_t size, const char *host)
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

// double coord(double x,double y,double d,double rotate,double *x1,double *y1)
// {
//     *x1=x+d*cos(rotate);
//     *y1=y+d*sin(rotate);
// }

// void latLonToGaussKruger(double dLat, double dLon, double *X, double *Y) {
//     int zone = (int)(dLon / 6.0 + 1);
//     // Параметры эллипсоида Красовского
//     double e2 = (A * A - B * B) / (A * A);  // Эксцентриситет
//     double n = (A - B) / (A + B);            // Приплюснутость

//     // Параметры зоны Гаусса-Крюгера
//     double F = 1.0;                           // Масштабный коэффициент
//     double Lat0 = 0.0;                        // Начальная параллель (в радианах)
//     double Lon0 = (zone * 6 - 3) * M_PI / 180; // Центральный меридиан (в радианах)
//     double N0 = 0.0;                          // Условное северное смещение для начальной параллели
//     double E0 = zone * 1e6 + 500000.0;       // Условное восточное смещение для центрального меридиана

//     // Перевод широты и долготы в радианы
//     double Lat = dLat * M_PI / 180.0;
//     double Lon = dLon * M_PI / 180.0;

//     // Вычисление переменных для преобразования
//     double v = A * F * pow(1 - e2 * pow(sin(Lat), 2), -0.5);
//     double p = A * F * (1 - e2) * pow(1 - e2 * pow(sin(Lat), 2), -1.5);
//     double n2 = v / p - 1;
    
//     double M1 = (1 + n + 5.0 / 4.0 * n * n + 5.0 / 4.0 * n * n * n) * (Lat - Lat0);
//     double M2 = (3 * n + 3 * n * n + 21.0 / 8.0 * n * n * n) * sin(Lat - Lat0) * cos(Lat + Lat0);
//     double M3 = (15.0 / 8.0 * n * n + 15.0 / 8.0 * n * n * n) * sin(2 * (Lat - Lat0)) * cos(2 * (Lat + Lat0));
//     double M4 = 35.0 / 24.0 * n * n * n * sin(3 * (Lat - Lat0)) * cos(3 * (Lat + Lat0));
    
//     double M = B * F * (M1 - M2 + M3 - M4);
//     double I = M + N0;
//     double II = v / 2 * sin(Lat) * cos(Lat);
//     double III = v / 24 * sin(Lat) * pow(cos(Lat), 3) * (5 - pow(tan(Lat), 2) + 9 * n2);
//     double IIIA = v / 720 * sin(Lat) * pow(cos(Lat), 5) * (61 - 58 * pow(tan(Lat), 2) + pow(tan(Lat), 4));
//     double IV = v * cos(Lat);
//     double V = v / 6 * pow(cos(Lat), 3) * (v / p - pow(tan(Lat), 2));
//     double VI = v / 120 * pow(cos(Lat), 5) * (5 - 18 * pow(tan(Lat), 2) + pow(tan(Lat), 4) + 14 * n2 - 58 * pow(tan(Lat), 2) * n2);

//     // Вычисление северного и восточного см
//     // Вычисление северного и восточного смещения (в метрах)
//     *X = I + II * pow((Lon - Lon0), 2) + III * pow((Lon - Lon0), 4) + IIIA * pow((Lon - Lon0), 6);
//     *Y = E0 + IV * (Lon - Lon0) + V * pow((Lon - Lon0), 3) + VI * pow((Lon - Lon0), 5);
// }

void generate_normal_distribution(double *values, int n)
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
        z0 = u1 * factor * SIGMA;
        z1 = u2 * factor * SIGMA;
        if (z0 < -LIMIT * SIGMA) z0 = -LIMIT * SIGMA;
        if (z0 > LIMIT * SIGMA) z0 = LIMIT * SIGMA;
        if (z1 < -LIMIT * SIGMA) z1 = -LIMIT * SIGMA;
        if (z1 > LIMIT * SIGMA) z1 = LIMIT * SIGMA;
        values[i++] = z0;
        if (i < n) values[i++] = z1;
    }
}

double model_fly(double *pitch, double *roll,double *yaw)
{
    *pitch=0.0;//тангаж м/с^2 ось Y
    *roll=0.0;;//крен ось X
    *yaw=9.81;//рысканье ось Z
}

int main(void)
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));
    double *test = (double *)malloc(NUM_SAMPLES * sizeof(double));
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_distribution(values, NUM_SAMPLES);
    // double lat=59.802977330951;
    // double lon= 30.26774253847127;
    // double X, Y;
    // double x1,y1;
    // double d=10000;
    double pitch;
    double roll;
    double yaw;
    model_fly(&pitch,&roll,&yaw);
    double Yaw=yaw;
    double Pitch=pitch;
    double Roll=roll;
    printf("Время:\tУскорение по оси X:\tУскорение по оси Y:\tУскорение по оси Z:\n");
    for(int i=0;i<NUM_SAMPLES;i++){
        pitch+=values[i];
        roll+=values[i];
        yaw+=values[i];
        test[i]=yaw;
        printf("  %d\t    %f\t            %f\t            %f\n",i,roll,pitch,yaw);
        sqlite3 *db;
        char *err_msg=0;
        int rc=sqlite3_open("Logs.db",&db);
        if(rc !=SQLITE_OK)
        {
        sqlite3_close(db);
        return 1;
        }   
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO Accelerometrs VALUES (%d,%f,%f,%f)", i, roll, pitch, yaw);
        rc=sqlite3_exec(db,sql,0,0,&err_msg);
        if(rc!=SQLITE_OK)
        {
            printf ("SQL error: %s\n",err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_close(db);
        yaw=Yaw;
        pitch=Pitch;
        roll=Roll;
    }
    char answer;
    printf("Хотите отправить данные для построения графика(y/n))?");
    scanf("%s",&answer);
    if(answer=='y')
    {
    size_t size=NUM_SAMPLES;
    if(size==NUM_SAMPLES)
    {
        send_array(test,size,SERVER_IP);
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
    // latLonToGaussKruger(lat, lon, &X, &Y);
    // printf("Широта: %.12f\n", lat);
    // printf("Долгота: %.12f\n", lon);
    // printf("X: %.12f\n", X);
    // printf("Y: %.12f\n", Y);
    // coord(X,Y,d,rotation_angle,&x1,&y1);
    // printf("X1: %f\nY1: %f\n",x1,y1);
    return 0;   
}