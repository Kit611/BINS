#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER_SIZE 1024 
#define NUM_SAMPLES 900 
#define SIGMA 0.025 
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

void generate_normal_distribution(double *values, int n) {
    int i = 0;
    while (i < n) {
        double u1, u2, s, z0, z1;

        do {
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

float model_fly()
{
    float h=1000;
    return h;
}

int main(void)
{
    srand(time(NULL));
    double *values = (double *)malloc(NUM_SAMPLES * sizeof(double));
    if (values == NULL) 
    {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    generate_normal_distribution(values, NUM_SAMPLES);
    float real_h=model_fly();
    float P;
    float p;
    float* sys_er;
    const float P_0=1013.25;
    const int H=8400;
    float stepen=real_h/8400;
    float e=exp(-stepen);
    P=P_0*e;
    p=P;
    printf("Введите погрешность от -4.5 до 4.5: \n");
    scanf("%F",sys_er);
    P+=*sys_er;
    printf("Время:\tРеальная высота(м):\tРеальная высота(mbar):\tДанные на выход с барометра:\n"); 
    for(int i=0;i<900;i++){
        P+=values[i];
        printf("  %d\t  %f\t             %f\t              %f\n",i,real_h,p,P);
    }
    size_t size=NUM_SAMPLES;
    if(size==900)
    {
        send_array(values,size,SERVER_IP);
        printf("%s","Отправлено");
    }
    else
    {
        printf("Количество элементов: %ld", size);
    }
    free(values);
    return 0;
}

//начальные координаты->> движение вверх ->> скорость набора высоты 3м/с ->> от 0с до 10с, расстояние. Через запрос