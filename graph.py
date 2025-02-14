import pandas as pd
import matplotlib.pyplot as plt
import sqlite3

while(True):
    a=int(input('Выберите таблицу(1-начальные данные или 2-конечные данные):'))
    if a==1:
        table='model_flight'
        height='H_m'
        vx='Vx_mSec'
        vy='Vy_mSec'
        vz='Vz_mSec'
        ox='Ox_C'
        oy='Oy_C'
        oz='Oz_C'
        vox='Vox_cSec'
        voy='Voy_cSec'
        voz='Voz_cSec'
        ax='ax_m2Sec'
        ay='ay_m2Sec'
        az='az_m2Sec'
        bx='Bx_G'
        by='By_G'
        bz='Bz_G'
        break
    elif a==2:
        table='finish_data'
        height='H_mbar'
        vx='Vx_mSec'
        vy='Vy_mSec'
        vz='Vz_mSec'
        ox='Ox_C'
        oy='Oy_C'
        oz='Oz_C'
        vox='Vox_cSec'
        voy='Voy_cSec'
        voz='Voz_cSec'
        ax='ax_m2Sec'
        ay='ay_m2Sec'
        az='az_m2Sec'
        bx='Bx_mG'
        by='By_mG'
        bz='Bz_mG'
        break
    else:
        print("Нет такой таблицы") 

# Подключение к базе данных
engine = sqlite3.connect('Logs.db')
# Извлечение данных из таблицы
query = f"SELECT * FROM {table}"
data = pd.read_sql(query, engine)
plt.figure(figsize=(24, 12))

# Построение графиков
#1. Координаты
plt.subplot(2, 4, 1)
plt.plot(data['Time_sec'], data['X'], label='Coordinate X', color='black')
plt.plot(data['Time_sec'], data['Y'], label='Coordinate Y', color='blue')
plt.title('Coordinate')
plt.xlabel('Time')
plt.ylabel('Coordinate')
plt.legend()
plt.grid()

# 2. Высота
plt.subplot(2, 4, 2)
plt.plot(data['Time_sec'], data[height], label='Height', color='blue')
plt.title('Barometer')
plt.xlabel('Time')
plt.ylabel('Height')
plt.grid()

# 3. Угол поворота
plt.subplot(2, 4, 3)
plt.plot(data['Time_sec'], data[ox], label='Angle X', color='purple')
plt.plot(data['Time_sec'], data[oy], label='Angle Y', color='brown')
plt.plot(data['Time_sec'], data[oz], label='Angle Z', color='pink')
plt.title('Gyroscopes')
plt.xlabel('Time')
plt.ylabel('Angle')
plt.legend()
plt.grid()

#4. Угловая скорость
plt.subplot(2, 4, 4)
plt.plot(data['Time_sec'], data[vox], label='Angular velocity X', color='purple')
plt.plot(data['Time_sec'], data[voy], label='Angular velocity Y', color='brown')
plt.plot(data['Time_sec'], data[voz], label='Angular velocity Z', color='pink')
plt.title('Gyroscopes')
plt.xlabel('Time')
plt.ylabel('Angular velocity')
plt.legend()
plt.grid()

#4. Скорость
plt.subplot(2, 4, 5)
plt.plot(data['Time_sec'], data[vx], label='Velocity X', color='red')
plt.plot(data['Time_sec'], data[vy], label='Velocity Y', color='green')
plt.plot(data['Time_sec'], data[vz], label='Velocity Z', color='orange')
plt.title('Accelerometers')
plt.xlabel('Time')
plt.ylabel('Velocity')
plt.legend()
plt.grid()

# 6. Ускорение по осям
plt.subplot(2, 4, 6)
plt.plot(data['Time_sec'], data[ax], label='Acceleration X', color='red')
plt.plot(data['Time_sec'], data[ay], label='Acceleration Y', color='green')
plt.plot(data['Time_sec'], data[az], label='Acceleration Z', color='orange')
plt.title('Accelerometers')
plt.xlabel('Time')
plt.ylabel('Acceleration')
plt.legend()
plt.grid()

# 7. Магнитное поле по осям
plt.subplot(2, 4, 7)
plt.plot(data['Time_sec'], data[bx], label='Magnetic X', color='cyan')
plt.plot(data['Time_sec'], data[by], label='Magnetic Y', color='magenta')
plt.plot(data['Time_sec'], data[bz], label='Magnetic Z', color='yellow')
plt.title('Magnetometer')
plt.xlabel('Time')
plt.ylabel('Magnetic Field')
plt.legend()
plt.grid()

if a==2:
    #8. Магнитное склонение и наклонение
    plt.subplot(2, 4, 8)
    plt.plot(data['Time_sec'], data['declination_c'], label='Declination', color='black')
    plt.plot(data['Time_sec'], data['inclination_c'], label='Inclination', color='blue')
    plt.title('Magnetometer')
    plt.xlabel('Time')
    plt.ylabel('Magnetic')
    plt.legend()
    plt.grid()

# Показать графики
plt.tight_layout()
plt.show()