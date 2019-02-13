﻿// ConsoleApplication2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <string>
#include <iostream>
typedef struct
{
	uint8_t Hours;
	uint8_t Minutes;
}Lines;
typedef struct
{
	uint8_t Hours;
	uint8_t Minutes;
}STime;
Lines line[4];
STime sTime;
int get_sTimeLinesDiff(Lines* lineToCheck)
{
	//Если возвращается отрицательное число, то необходимо ждать возвращенное количество минут.
	//Если положительное, то необходимо добавить к времени на линиях
	int16_t diff_Min = 0;					//	Инициализация переменной для разницы в минутах. тип int - разница может быть положительной и отрицательной
											//диапазон значений от -32768 до 32767
	uint8_t sHour12 = 0, lHour12 = 0;		//	Инициализация переменных, в которых хранятся значения системного времени (sHour12) и времени на линии (lHour12)
											//в 12 часовом формате. Диапазон значений: 0..255
	sHour12 = sTime.Hours % 12;				//	Берем остаток от деления количества часов системного времени на 12. 
											//Пример: 1%12=1,2%12=2... 12%12=0, 13%12=1,14%12=2 ... то есть переводим 24ч в 12ч формат
	if (sHour12 == 0)						//Если после перевода в 12ч формат получился 0
	{
		sHour12 = 12;						//То заменяем его на 12ч, т.к. на циферблате нет 0ч
	}
	//Перевод в 12ч формат системного времени (sTime) завершен
	lHour12 = lineToCheck->Hours % 12;		//	Аналогично для  времени на контролируемой линии. Только доступ через указатель на lineToCheck
	if (lHour12 == 0)
	{
		lHour12 = 12;
	}
	//Перевод в 12ч формат времени на линии завершен
	//Вычисляем разницу: 
	//			часы * 60мин + минуты       -   ......
	//Пример.Сис.время= 10:20, время на линии 20:00. 
	//После перевода в 12ч: Сис.время 10:20, на линии 08:00
	//diff_Min = 10*60 + 20 - (8*60+0) = 620-480=140 минут (импульсов).
	//Если число положительное, то значит системное время в 12ч формате опережает время на линии в 12ч формате, и можно спокойно выдать 140 импульсов
	//Тем самым стрелки на часах перейдут из 08:00 (20:00) в 10:20, но в 24ч это будет либо 10:20, либо 22:20.
	//Если число отрицтальное, значит наоборот. Системное время отстает от времени на линии. И, в принципе, можно ждать пока оно догонет, и после этого
	//начать выдавать импульсы. Но ждать можно и час, и два, и 11 часов. 
	diff_Min = sHour12 * 60 + sTime.Minutes - (lHour12 * 60 + lineToCheck->Minutes);

	if (diff_Min < -10) //Если число больше меньше -10, то есть если системное время в 12ч формате отстает от времени на линии в 12ч более чем на 10 минут
	{
		//Если все же ждать очень долго (более 10 минут)
		diff_Min = 720 + diff_Min;         //крутим стрелки вперед на 720 минут (12 часов) минус разница. То есть, 720-120, например. 6 часов.
	}
	//иначе просто выдаем разницу. Предполагается, что если функция возвращает отрицательное число (а оно может быть исходя из предыдущего условия от -10 до -1),
	//то далее программа должна ничего не делать именно столько времени

	return diff_Min;
}

int main()
{
	int32_t out = 0;
	bool again = true;
	char in[6];
	std::string strMinutes = "";
	setlocale(LC_ALL, "rus");
	while (again == true)
	{
		strMinutes = "";
		std::cout << "Время на линии ЧЧ:ММ\r\n";
		std::cin >> in; //вводим время на линии

		line[0].Hours = (in[0] - 0x30) * 10 + (in[1] - 0x30); //магия перевода строки в число
		line[0].Minutes = (in[3] - 0x30) * 10 + (in[4] - 0x30); //продолжение магии
		std::cout << "\r\nСистемное время ЧЧ:ММ\r\n";
		std::cin >> in; //вводим системное время
		sTime.Hours = (in[0] - 0x30) * 10 + (in[1] - 0x30); //очередная магия
		sTime.Minutes = (in[3] - 0x30) * 10 + (in[4] - 0x30); //...
		out = get_sTimeLinesDiff(&line[0]); //переменная out равна возвращаемому значению из функции, в которую передается line[0] для контроля
		if (out > 0)
		{
			line[0].Hours = (line[0].Hours + abs(out) / 60) % 24;
		}

		if (line[0].Hours > 12 && sTime.Hours < 12)
		{
			line[0].Hours -= 12;
		}
		else
			if (line[0].Hours < 12 && sTime.Hours > 12)
			{
				line[0].Hours -= 12;
			}
			else
				if (line[0].Hours == 0 && sTime.Hours == 12)
				{
					line[0].Hours == 12;
				}
				else
					if (line[0].Hours == 12 && sTime.Hours == 0)
					{
						line[0].Hours == 0;
					}

		line[0].Minutes += out % 60;
		if (line[0].Minutes >= 60)
		{
			line[0].Hours = (line[0].Hours + line[0].Minutes / 60) % 24;
			line[0].Minutes = line[0].Minutes % 60;
		}
		if (line[0].Minutes < 10)
		{
			strMinutes.append("0");
			strMinutes.append(std::to_string(line[0].Minutes));
		}
		else
		{
			strMinutes = std::to_string(line[0].Minutes);
		}
		std::cout << "Результат после работы функции: " << std::to_string(line[0].Hours) << ":" << strMinutes << "\r\n" << "Еще?(1/0)\r\n";
		std::cin >> in;
		std::cout << "\r\n";
		again = in[0] - 0x30;
	}
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
