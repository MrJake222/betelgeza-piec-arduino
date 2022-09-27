


#include <LittleFS.h>
void listAllFilesInDir(String dir_path, int level = 0);

#include <time.h>
String get_date_time();
uint16_t get_wday();
uint16_t tech_get_time();
String tech_time_to_string(uint16_t time);
int tech_time_diff(int time, int time2);