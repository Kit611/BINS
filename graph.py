import pandas as pd
import matplotlib.pyplot as plt
import sqlite3

# Подключение к базе данных
# Замените 'sqlite:///your_database.db' на вашу строку подключения
engine = sqlite3.connect('Logs.db')

# Извлечение данных из таблицы
# Замените 'your_table_name' на имя вашей таблицы
query = "SELECT * FROM model_flight"
data = pd.read_sql(query, engine)

# Пример: вывод первых нескольких строк данных
print(data.head())

# Построение графиков
# 1. Высота
plt.figure(figsize=(12, 6))
plt.subplot(2, 2, 1)
plt.plot(data['Time_sek'], data['Height_m'], label='Height', color='blue')
plt.title('Height over Time')
plt.xlabel('Time')
plt.ylabel('Height')
plt.grid()

# 2. Ускорение по осям
plt.subplot(2, 2, 2)
plt.plot(data['Time_sek'], data['X_axis_acceleration_m2sek'], label='Acceleration X', color='red')
plt.plot(data['Time_sek'], data['Y_axis_acceleration_m2Sek'], label='Acceleration Y', color='green')
plt.plot(data['Time_sek'], data['Z_axis_acceleration_m2sek'], label='Acceleration Z', color='orange')
plt.title('Acceleration over Time')
plt.xlabel('Time')
plt.ylabel('Acceleration')
plt.legend()
plt.grid()

# 3. Угловая скорость по осям
plt.subplot(2, 2, 3)
plt.plot(data['Time_sek'], data['Roll_gradsek'], label='Angular Velocity X', color='purple')
plt.plot(data['Time_sek'], data['Pitch_gradsek'], label='Angular Velocity Y', color='brown')
plt.plot(data['Time_sek'], data['Yaw_gradsek'], label='Angular Velocity Z', color='pink')
plt.title('Angular Velocity over Time')
plt.xlabel('Time')
plt.ylabel('Angular Velocity')
plt.legend()
plt.grid()

# 4. Магнитное поле по осям
plt.subplot(2, 2, 4)
plt.plot(data['Time_sek'], data['X_nT'], label='Magnetic Field X', color='cyan')
plt.plot(data['Time_sek'], data['Y_nT'], label='Magnetic Field Y', color='magenta')
plt.plot(data['Time_sek'], data['Z_nT'], label='Magnetic Field Z', color='yellow')
plt.title('Magnetic Field over Time')
plt.xlabel('Time')
plt.ylabel('Magnetic Field')
plt.legend()
plt.grid()

# Показать графики
plt.tight_layout()
plt.show()