#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double normal_random(double mean, double stddev)
{
    double u1 = ((double) rand() / RAND_MAX); 
    double u2 = ((double) rand() / RAND_MAX);
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return z0 * stddev + mean;
}

double bounded_normal_random(double mean, double stddev, double min, double max)
{
    double value;
    do {
        value = normal_random(mean, stddev);
    } while (value < min || value > max);
    return value;
}

float model_fly()
{
    float h=1000;
    return h;
}

int main(void)
{
    srand(time(NULL));
    double mean = 0.045;
    double stddev = 0.025;
    double min = -0.009;
    double max = 0.009;  
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
    // printf("Введите погрешность от -4.5 до 4.5: \n");
    // scanf("%F",sys_er);
    P+=*sys_er;
    p=P;
    // printf("Время:\tРеальная высота(м):\tРеальная высота(mbar):\tДанные на выход с барометра:\n"); 
    for(int i=0;i<900;i++)
    {
        double noise = bounded_normal_random(mean, stddev, min, max);          
        printf("%f,",noise);
        // P+=noise;
        // printf("  %d\t    %.1f\t           %f\t             %f\n",i,real_h,p,P);
        // P=p;
    }
    return 0;
}

//начальные координаты->> движение вверх ->> скорость набора высоты 3м/с ->> от 0с до 10с, расстояние. Через запрос

//min =0, max=0,9, sigma=0.25, шаг=0,01, число реализаций =900, 