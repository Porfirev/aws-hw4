#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <chrono>
#include <map>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>


int now_ans;
int now_ticket_num;
// mutex на запись
pthread_mutex_t write_mutex;
// mutex на чтение
pthread_mutex_t read_mutex;
std::string name;
std::map<std::string, int> tickets_num;
// Кол-во ужедших из аудитории
int count_departed = 0;
// генерация рандома
int gen() {
	return abs(rand());
}

// проверка прподавателем(сервером) ответа
bool check_ans() {
	// Время проверки
	sleep(1);
	// Условность, что правельный ответ - это номер билета
  	return (now_ans == now_ticket_num);
}

// функция преподавателя (сервера)
void *server(void *param) {
	while (true) {
		pthread_mutex_lock(&read_mutex);
	  	pthread_mutex_unlock(&write_mutex);
	  	std::string student_info = "Студент: " + name;
	  	std::string res;
	  	// Печать результата (сдал или не сдал билет)
	  	if (check_ans()) {
	  		res = student_info + " - сдал билет номер " + std::to_string(now_ticket_num);
	  	} else {
			res = student_info + " - не сдал билет номер " + std::to_string(now_ticket_num);
	  	}
    	std::cout << res << std::endl;
    	count_departed++;
  	}
  	return nullptr;
}

// функция студента (клиента)
void *client(void *param) {
	// Распаковка имени
	tickets_num[*(std::string *)param] = (abs(gen()) % 100);
	sleep(abs(gen()) % 3);
	pthread_mutex_lock(&write_mutex);
	name = *(std::string *)param;
	now_ticket_num = tickets_num[name];
	now_ans = now_ticket_num * (abs(gen()) % 3);
	pthread_mutex_unlock(&read_mutex);
	return nullptr;
}

int main(int argc, char** argv) {
	srand(time(0)); 
	int count_students;
	std::vector<std::string> names;
	pthread_mutex_init(&(read_mutex), nullptr);
	pthread_mutex_init(&(write_mutex), nullptr);
	pthread_mutex_lock(&read_mutex);
	if (argc == 1) {
		std::cout << "Количество студентов: ";
		std::cin >> count_students;
		std::cout << "\nИмена студентов:\n";
		
		for (int i = 0; i < count_students; ++i) {	
			std::cin >> name;
			names.push_back(name);
		}
	} else if ((std::string)argv[1] != "file") {
		count_students = argc - 1;
		for (int i = 1; i < argc; ++i) {
			names.push_back(std::string(argv[i]));
		}
	} else {
		std::ifstream fin((std::string)argv[2]);
		fin >> count_students;
		for (int i = 0; i < count_students; ++i) {	
			fin >> name;
			names.push_back(name);
		}
		fin.close();
	}
	pthread_t students[count_students];
	pthread_t teacher;
	pthread_create(&teacher, nullptr, server, nullptr);
	for (int i = 0; i < count_students; ++i) {
		pthread_create(&students[i], nullptr, client, (void *)(&names[i]));
	}
	while (count_departed != count_students) {}
}
