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
#include <stdio.h>
#include <math.h>

#define DEG_TO_RAD (M_PI / 180.0)
#define G_TO_UT 100.0   // 1 Гаусс (G) = 100 микротесла (µT)
#define G_TO_MG 10.0    // 1 Гаусс (G) = 10 миллигауссов (mG)

// Магнитное поле Земли в NED (Москва) в Гауссах (G)
double B_ned_g[3] = {0.14451, 0.03056, 0.50904};

// Функция расчета матрицы поворота (yaw, pitch, roll в градусах)
void compute_rotation_matrix(double yaw, double pitch, double roll, double R[3][3]) {
    double cy = cos(yaw * DEG_TO_RAD), sy = sin(yaw * DEG_TO_RAD);
    double cp = cos(pitch * DEG_TO_RAD), sp = sin(pitch * DEG_TO_RAD);
    double cr = cos(roll * DEG_TO_RAD), sr = sin(roll * DEG_TO_RAD);

    // Матрица поворота из NED в систему вертолета
    R[0][0] = cy * cp;
    R[0][1] = cy * sp * sr - sy * cr;
    R[0][2] = cy * sp * cr + sy * sr;
    R[1][0] = sy * cp;
    R[1][1] = sy * sp * sr + cy * cr;
    R[1][2] = sy * sp * cr - cy * sr;
    R[2][0] = -sp;
    R[2][1] = cp * sr;
    R[2][2] = cp * cr;
}

// Применение матрицы поворота к вектору магнитного поля
void transform_magnetic_field(double R[3][3], double B_ned[3], double B_lcs[3]) {
    for (int i = 0; i < 3; i++) {
        B_lcs[i] = R[i][0] * B_ned[0] + R[i][1] * B_ned[1] + R[i][2] * B_ned[2];
    }
}

int main() {
    double yaw = 0, pitch = -30, roll = 0;  // Вертолет висит без наклонов
    double altitude = 150;  // Высота 150 м

    double R[3][3], B_lcs_g[3], B_lcs_ut[3], B_lcs_mg[3];

    // Рассчитываем матрицу поворота
    compute_rotation_matrix(yaw, pitch, roll, R);

    // Преобразуем магнитное поле в систему вертолета (в Гауссах)
    transform_magnetic_field(R, B_ned_g, B_lcs_g);

    // Конвертация в µT и mG
    for (int i = 0; i < 3; i++) {
        B_lcs_ut[i] = B_lcs_g[i] * G_TO_UT; // G → µT
        B_lcs_mg[i] = B_lcs_g[i] * G_TO_MG; // G → mG
    }

    // Вывод результата
    printf("Высота: %.1f м\n", altitude);
    printf("Измерения магнитометра:\n");

    printf("В Гауссах (G):       Bx = %.4f, By = %.4f, Bz = %.4f\n",
           B_lcs_g[0], B_lcs_g[1], B_lcs_g[2]);

    printf("В микротеслах (µT):  Bx = %.2f, By = %.2f, Bz = %.2f\n",
           B_lcs_ut[0], B_lcs_ut[1], B_lcs_ut[2]);

    printf("В миллигауссах (mG): Bx = %.2f, By = %.2f, Bz = %.2f\n",
           B_lcs_mg[0], B_lcs_mg[1], B_lcs_mg[2]);

    return 0;
}