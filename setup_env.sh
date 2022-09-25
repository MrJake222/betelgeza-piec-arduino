
# Add esp8266 URL & install board
ESP_URL="http://arduino.esp8266.com/stable/package_esp8266com_index.json"
arduino-cli --additional-urls ${ESP_URL} update
arduino-cli --additional-urls ${ESP_URL} core install esp8266:esp8266

# Install libs NOT WORKING NOT NEEDED??
#arduino-cli lib update-index
#LIBS="LittleFS ESP8266WiFi ESP8266WebServer SoftwareSerial"
#for lib in ${LIBS}; do
	#echo "Installing ${lib}"
	#arduino-cli lib install ${lib}
#done
