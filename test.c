#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Функция для генерации нормально распределённого случайного числа
double normal_random(double mean, double stddev) {
    double u1 = ((double) rand() / RAND_MAX); // Генерация первого равномерного случайного числа
    double u2 = ((double) rand() / RAND_MAX); // Генерация второго равномерного случайного числа

    // Применение метода Box-Muller
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return z0 * stddev + mean; // Преобразование в нормальное распределение
}

// Функция для генерации нормально распределённого числа в заданном диапазоне
double bounded_normal_random(double mean, double stddev, double min, double max) {
    double value;

    do {
        value = normal_random(mean, stddev);
    } while (value < min || value > max); // Повторяем, пока значение не попадает в диапазон

    return value;
}

int main() {
    // Инициализация генератора случайных чисел
    srand(time(NULL));

    double mean = 0.0045;    // Среднее значение, чтобы центрировать распределение
    double stddev = 0.025;   // Стандартное отклонение
    double min = 0.0;        // Нижняя граница
    double max = 0.009;      // Верхняя граница

    // Генерация и вывод 10 нормально распределённых случайных чисел в диапазоне [0, 0.009]
    for (int i = 0; i < 10; i++) {
        double random_value = bounded_normal_random(mean, stddev, min, max);
        printf("%f\n", random_value);
    }

    return 0;
}
