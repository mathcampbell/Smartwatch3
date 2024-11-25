#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <JSON_Decoder.h>
#include <OpenWeather.h>
#include <Time.h>




// Declare WeatherData structure
struct WeatherData {
    String temperature;
    String condition;
    String icon;
    String sunrise;
    String sunset;
    String wind_speed;
    String humidity;
     unsigned long lastUpdate; 
    uint16_t id;
    unsigned long dt;
};

// Declare external variables
extern String api_key;
extern String latitude;
extern String longitude;
extern String units;
extern String language;
extern WeatherData currentWeatherData;

// Declare functions
void WeatherInit();
void printCurrentWeather();
void updateWeatherData();
const char* getMeteoconIcon(uint16_t id, bool today);
bool loadWeatherDataFromFile(const char* filePath, WeatherData& weather);
void saveWeatherDataToFile(const char* filePath, const WeatherData& weather);
void initializeWeatherData();

String strTime(time_t unixTime);

#endif // WEATHER_MANAGER_H