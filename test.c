// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <time.h>

// #define NUM_SAMPLES 900 // Количество случайных чисел
// #define SIGMA 0.025        // Стандартное отклонение
// #define LIMIT 3.0          // Ограничение в 3 сигмы

// // Генерация случайных чисел с нормальным распределением (Polar Form)
// void generate_normal_distribution(double *values, int n) {
//     int i = 0;
//     while (i < n) {
//         double u1, u2, s, z0, z1;

//         do {
//             u1 = ((double)rand() / RAND_MAX) * 2.0 - 1.0; // от -1 до 1
//             u2 = ((double)rand() / RAND_MAX) * 2.0 - 1.0; // от -1 до 1
//             s = u1 * u1 + u2 * u2;
//         } while (s >= 1 || s == 0); // s должно быть в (0,1)

//         double factor = sqrt(-2.0 * log(s) / s);
//         z0 = u1 * factor * SIGMA;
//         z1 = u2 * factor * SIGMA;

//         // Ограничиваем значения в диапазоне [-3σ, 3σ]
//         if (z0 < -LIMIT * SIGMA) z0 = -LIMIT * SIGMA;
//         if (z0 > LIMIT * SIGMA) z0 = LIMIT * SIGMA;
//         if (z1 < -LIMIT * SIGMA) z1 = -LIMIT * SIGMA;
//         if (z1 > LIMIT * SIGMA) z1 = LIMIT * SIGMA;

//         // Добавляем в массив (используем два значения за итерацию)
//         values[i++] = z0;
//         if (i < n) values[i++] = z1;
//     }
// }

// int main() {
//     srand(time(NULL));  // Инициализация генератора случайных чисел

//     double values[NUM_SAMPLES];

//     generate_normal_distribution(values, NUM_SAMPLES);

//     // Запись данных в файл для анализа в Python
//     FILE *file = fopen("data.txt", "w");
//     if (file == NULL) {
//         printf("Ошибка при открытии файла!\n");
//         return 1;
//     }
//     for (int i = 0; i < NUM_SAMPLES; i++) {
//         fprintf(file, "%f\n", values[i]);
//     }
//     fclose(file);

//     // Вывод первых 10 значений для проверки
//     printf("Первые 10 значений нормального распределения (с ограничением в 3 сигмы):\n");
//     for (int i = 0; i < 10; i++) {
//         printf("%f\n", values[i]);
//     }

//     printf("\nДанные сохранены в 'data.txt'.\n");

//     return 0;
// }
// #include <stdio.h>
// #include <math.h>

// void calculate_magnetic_parameters(double B_north, double B_east, double B_down, double *B_horizontal, double *B_total, double *azimuth) {
//     *B_horizontal = sqrt(B_north * B_north + B_east * B_east);
//     *B_total = sqrt(B_north * B_north + B_east * B_east + B_down * B_down);
//     *azimuth = atan2(B_east, B_north);
// }

// int main() {
//     double B_north = 0.1294*1000;  // Замените на фактические значения
//     double B_east = 0.0306*1000;
//     double B_down = 0.5131*1000;//, By = , Bz =

//     double B_horizontal, B_total, azimuth;
//     calculate_magnetic_parameters(B_north, B_east, B_down, &B_horizontal, &B_total, &azimuth);
//     double azimuth_c=azimuth*(180/M_PI);

//     printf("Горизонтальная составляющая: %f\n", B_horizontal);
//     printf("Общее значение магнитного поля: %f\n", B_total);
//     printf("Азимут магнитного поля: %f радиан\n", azimuth);
//     printf("Азимут магнитного поля: %f градус\n", azimuth_c);

//     return 0;
// }
// #include <stdio.h>
// #include <math.h>

// #define DEG_TO_RAD (M_PI / 180.0)
// #define G_TO_UT 100.0   // 1 Гаусс (G) = 100 микротесла (µT)
// #define G_TO_MG 10.0    // 1 Гаусс (G) = 10 миллигауссов (mG)

// // Магнитное поле Земли в NED (Москва) в Гауссах (G)
// double B_ned_g[3] = {0.14451, 0.03056, 0.50904};

// // Функция расчета матрицы поворота (yaw, pitch, roll в градусах)
// void compute_rotation_matrix(double yaw, double pitch, double roll, double R[3][3]) {
//     double cy = cos(yaw * DEG_TO_RAD), sy = sin(yaw * DEG_TO_RAD);
//     double cp = cos(pitch * DEG_TO_RAD), sp = sin(pitch * DEG_TO_RAD);
//     double cr = cos(roll * DEG_TO_RAD), sr = sin(roll * DEG_TO_RAD);

//     // Матрица поворота из NED в систему вертолета
//     R[0][0] = cy * cp;
//     R[0][1] = cy * sp * sr - sy * cr;
//     R[0][2] = cy * sp * cr + sy * sr;
//     R[1][0] = sy * cp;
//     R[1][1] = sy * sp * sr + cy * cr;
//     R[1][2] = sy * sp * cr - cy * sr;
//     R[2][0] = -sp;
//     R[2][1] = cp * sr;
//     R[2][2] = cp * cr;
// }

// // Применение матрицы поворота к вектору магнитного поля
// void transform_magnetic_field(double R[3][3], double B_ned[3], double B_lcs[3]) {
//     for (int i = 0; i < 3; i++) {
//         B_lcs[i] = R[i][0] * B_ned[0] + R[i][1] * B_ned[1] + R[i][2] * B_ned[2];
//     }
// }

// int main() {
//     double yaw = 0, pitch = -30, roll = 0;  // Вертолет висит без наклонов
//     double altitude = 150;  // Высота 150 м

//     double R[3][3], B_lcs_g[3], B_lcs_ut[3], B_lcs_mg[3];

//     // Рассчитываем матрицу поворота
//     compute_rotation_matrix(yaw, pitch, roll, R);

//     // Преобразуем магнитное поле в систему вертолета (в Гауссах)
//     transform_magnetic_field(R, B_ned_g, B_lcs_g);

//     // Конвертация в µT и mG
//     for (int i = 0; i < 3; i++) {
//         B_lcs_ut[i] = B_lcs_g[i] * G_TO_UT; // G → µT
//         B_lcs_mg[i] = B_lcs_g[i] * G_TO_MG; // G → mG
//     }

//     // Вывод результата
//     printf("Высота: %.1f м\n", altitude);
//     printf("Измерения магнитометра:\n");

//     printf("В Гауссах (G):       Bx = %.4f, By = %.4f, Bz = %.4f\n",
//            B_lcs_g[0], B_lcs_g[1], B_lcs_g[2]);

//     printf("В микротеслах (µT):  Bx = %.2f, By = %.2f, Bz = %.2f\n",
//            B_lcs_ut[0], B_lcs_ut[1], B_lcs_ut[2]);

//     printf("В миллигауссах (mG): Bx = %.2f, By = %.2f, Bz = %.2f\n",
//            B_lcs_mg[0], B_lcs_mg[1], B_lcs_mg[2]);

//     return 0;
// }
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>

// int main()
// {
//     srand(time(NULL));
//     int ch = 1 + rand() % 15;
//     int ch_g = ch * 1000000;
//     float mag_g = (ch_g / 24);
//     float bar_g = (ch_g / 48);
//     float mag = mag_g / 1000000;
//     float bar = bar_g / 1000000;
//     printf("%d\n", ch);
//     printf("%d\n", ch_g);
//     printf("%.4f\n", mag);
//     printf("%.4f\n", bar);
//===============================
// int ch = 1 + rand() % 15;
// int ch_g = ch * 1000000;
// float mag_g = (ch_g / 24);
// float bar_g = (ch_g / 48);
// float period_ns = (long)(1e9 / ch_g); // 1 секунда = 1e9 наносекунд
// float period_s = period_ns / 1000000000;
// float bar_period_ns = (long)(1e9 / bar_g);
// float bar_period_s = bar_period_ns / 1000000000;
// float mag_period_ns = (long)(1e9 / mag_g);
// float mag_period_s = mag_period_ns / 1000000000;
// struct timespec last_update;
// clock_gettime(CLOCK_MONOTONIC, &last_update);
//=======================================
// }
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_SENSORS 4

// Структура для хранения информации о датчике
typedef struct
{
    char name[20];
    double frequency; // Частота в Гц
    long period_ns;   // Период в наносекундах
} Sensor;

// Функция для генерации данных датчика
double generate_data(Sensor sensor)
{
    // Генерация случайных данных (например, от 0 до 100)
    return (double)(rand() % 101);
}

// Функция для отображения данных
void display_data(Sensor sensor)
{
    double data = generate_data(sensor);
    printf("Датчик: %s, Частота: %.2f Гц, Данные: %.2f\n",
           sensor.name, sensor.frequency, data);
}

int main()
{
    Sensor sensors[NUM_SENSORS] = {
        {"Датчик 1", 2.0},  // 2 Гц (период 500000000 наносекунд)
        {"Датчик 2", 5.0},  // 5 Гц (период 200000000 наносекунд)
        {"Датчик 3", 10.0}, // 10 Гц (период 100000000 наносекунд)
        {"Датчик 4", 1.0}   // 1 Гц (период 1000000000 наносекунд)
    };

    // Вычисляем период для каждого датчика в наносекундах
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        sensors[i].period_ns = (long)(1e9 / sensors[i].frequency); // 1 секунда = 1e9 наносекунд
    }

    // Время последнего обновления для каждого датчика
    struct timespec last_update[NUM_SENSORS];
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        clock_gettime(CLOCK_MONOTONIC, &last_update[i]);
    }

    while (1)
    {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time); // Получаем текущее время

        // Проверяем каждый датчик
        for (int i = 0; i < NUM_SENSORS; i++)
        {
            long elapsed_ns = (current_time.tv_sec - last_update[i].tv_sec) * 1e9 +
                              (current_time.tv_nsec - last_update[i].tv_nsec);

            // Если прошло достаточно времени, чтобы обновить данные
            if (elapsed_ns >= sensors[i].period_ns)
            {
                display_data(sensors[i]);      // Показываем данные от датчика
                last_update[i] = current_time; // Обновляем время последнего обновления
            }
        }

        // Задержка для предотвращения чрезмерной загрузки процессора
        // struct timespec sleep_time;
        // sleep_time.tv_sec = 0;
        // sleep_time.tv_nsec = 100000; // Задержка на 100 микросекунд
        // nanosleep(&sleep_time, NULL);
    }

    return 0;
}