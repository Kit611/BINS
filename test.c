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

// Функция интегрирования для получения скорости из ускорения
double integrate_acceleration_to_velocity(double acceleration, double t_start, double t_end) {
    // Расчет времени
    double dt = t_end - t_start;

    // Начальная скорость (можно изменить по необходимости)
    double initial_velocity = 0; // Предположим, что начальная скорость равна 0

    // Скорость = начальная скорость + ускорение * время
    double final_velocity = initial_velocity + acceleration * dt;

    return final_velocity;
}

int main() {
    double acceleration;
    double t_start, t_end;

    // Запрашиваем у пользователя значение ускорения и временные точки
    printf("Введите значение ускорения: ");
    scanf("%lf", &acceleration);
    printf("Введите начальное время (t_start): ");
    scanf("%lf", &t_start);
    printf("Введите конечное время (t_end): ");
    scanf("%lf", &t_end);

    // Вызываем функцию интегрирования
    double final_velocity = integrate_acceleration_to_velocity(acceleration, t_start, t_end);

    // Выводим результат
    printf("Конечная скорость: %.6f\n", final_velocity);

    return 0;
}
