#include "helpers.hpp"

void listAllFilesInDir(String dir_path, int level) {
	Dir dir = LittleFS.openDir(dir_path);

	while(dir.next()) {

		if (dir.isFile()) {
			// print files
            for (int i=0; i<level; i++) {
                Serial.print("  ");
            }

			Serial.println("f " + dir_path + dir.fileName());
		}

		if (dir.isDirectory()) {
			// print dirs
			Serial.println("D " + dir_path + dir.fileName() + "/");
			// recursive file listing inside new directory
			listAllFilesInDir(dir_path + dir.fileName() + "/", level+1);
		}
	}
}

String get_date_time() {
    time_t now;
    tm tm;
    time(&now);
    localtime_r(&now, &tm);

    const char MONTHS[12][4] = { "Sty", "Lut", "Mar", "Kwi", "Maj", "Cze", "Lip", "Sie", "Wrz", "Paz", "Lis", "Gru" };
    const char WDAYS[7][6] = { "Niedz", "Pon", "Wt", "Sr", "Czw", "Pt", "Sob" };

    char buf[32];

    // Wt, 20 Sty 2022, 12:40:44
    sprintf(buf, "%s, %02d %s %d, %02d:%02d:%02d", WDAYS[tm.tm_wday], tm.tm_mday, MONTHS[tm.tm_mon], 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
    String date(buf);

    return date;
}