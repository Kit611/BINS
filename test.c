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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SAMPLE_COUNT 100

typedef struct {
    double mx; // Магнитное поле по оси X
    double my; // Магнитное поле по оси Y
    double mz; // Магнитное поле по оси Z
} SensorData;

// Генерация данных магнетометра
void simulate_magnetometer(double *x, double *y, double *z, int count) {
    for (int i = 0; i < count; i++) {
        double angle = (double)i / count * 2 * M_PI;
        x[i] = cos(angle) * 50000; // 50000 nT
        y[i] = sin(angle) * 50000; // 50000 nT
        z[i] = 0.5 * cos(angle / 2) * 50000; // 50000 nT
    }
}

// Функция для вычисления магнитного склонения
double calculate_declination(double mx, double my) {
    double declination = atan2(my, mx);
    return declination * (180.0 / M_PI); // Возвращаем угол склонения в градусах
}

// Функция для вычисления магнитного наклонения
double calculate_inclination(double mx, double my, double mz) {
    double inclination = atan2(mz, sqrt(mx * mx + my * my));
    return inclination * (180.0 / M_PI); // Возвращаем угол наклонения в градусах
}

int main() {
    SensorData data[SAMPLE_COUNT];
    double mx[SAMPLE_COUNT], my[SAMPLE_COUNT], mz[SAMPLE_COUNT];

    // Генерация данных
    simulate_magnetometer(mx, my, mz, SAMPLE_COUNT);

    // Заполнение структуры данных
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        data[i].mx = mx[i];
        data[i].my = my[i];
        data[i].mz = mz[i];
    }

    // Вычисление и вывод магнитного склонения и наклонения
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        double declination = calculate_declination(data[i].mx, data[i].my);
        double inclination = calculate_inclination(data[i].mx, data[i].my, data[i].mz);
        printf("Data %d: M(%f, %f, %f), Declination: %f degrees, Inclination: %f degrees\n", 
               i, data[i].mx, data[i].my, data[i].mz, declination, inclination);
    }

    return 0;
}