// IMPORTS //
#include <WiFi.h>
#include <JSON_Decoder.h>
#include <OpenWeather.h>
#include <Arduino.h>
#include <Time.h>
#include "WeatherManager.h"
#include "ui.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


// DEFINES //

#define TIME_OFFSET 1UL * 3600UL // UTC + 0 hour

// Change to suit your WiFi router
#define WIFI_SSID     "GraphicsForgeA"
#define WIFI_PASSWORD "25137916"

// VARIABLES //

// OpenWeather API Details, replace x's with your API key
String api_key = "3a31e88719b05f19b116b6acb55e883c"; // Obtain this from your OpenWeather account

// Set both your longitude and latitude to at least 4 decimal places
String latitude =  "56.0089507"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "-4.7990904"; // 180.000 to -180.000 negative for West

String units = "metric";  // or "imperial"
String language = "en";   // See notes tab

// NTP settings
const char* ntpServer = "pool.ntp.org";  // Use a public NTP server
const long utcOffsetInSeconds = 0;      // Adjust as necessary for your timezone
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);



// Main part starts here //
OW_Weather ow; // Weather forecast library instance

WeatherData currentWeatherData; // Global or instance variable

void WeatherInit() { 


    Serial.printf("\n\nConnecting to %s\n", WIFI_SSID);
    Serial.println("");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    delay(500);
  }

  Serial.println();
  Serial.println("Connected\n");
  Serial.println(WiFi.localIP());

   // Initialize NTP client
    Serial.println("Initializing NTP...");
    timeClient.begin();
    if (!timeClient.update()) {
        Serial.println("Failed to get time from NTP server.");
    } else {
        // Set the system time using NTP
        time_t currentTime = timeClient.getEpochTime();
        struct timeval tv = { .tv_sec = currentTime, .tv_usec = 0 };
        settimeofday(&tv, nullptr);
        Serial.println("Time synchronized with NTP:");
        Serial.printf("Current time: %s\n", ctime(&currentTime));
    }


  initializeWeatherData();
  
}

void saveWeatherDataToFile(const char* filePath, const WeatherData& weather) {
    File file = LittleFS.open(filePath, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    DynamicJsonDocument doc(512); // Adjust size as needed

    doc["temperature"] = weather.temperature;
    doc["condition"] = weather.condition;
    doc["icon"] = weather.icon;
    doc["sunrise"] = weather.sunrise;
    doc["sunset"] = weather.sunset;
    doc["wind_speed"] = weather.wind_speed;
    doc["humidity"] = weather.humidity;
    doc["lastUpdate"] = weather.lastUpdate;
    doc["id"] = weather.id;
    doc["dt"] = weather.dt;
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    }

    file.close();
    Serial.println("Weather data saved successfully");
}

bool loadWeatherDataFromFile(const char* filePath, WeatherData& weather) {
    File file = LittleFS.open(filePath, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }

    DynamicJsonDocument doc(512); // Adjust size as needed

    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Failed to read file: ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }

    weather.temperature = doc["temperature"].as<String>();
    weather.condition = doc["condition"].as<String>();
    weather.icon = doc["icon"].as<String>();
    weather.sunrise = doc["sunrise"].as<String>();
    weather.sunset = doc["sunset"].as<String>();
    weather.wind_speed = doc["wind_speed"].as<String>();
    weather.humidity = doc["humidity"].as<String>();
    weather.lastUpdate = doc["lastUpdate"].as<unsigned long>();
    weather.id = doc["id"].as<uint16_t>(); 
    weather.dt = doc["dt"].as<unsigned long>();

    file.close();
    Serial.println("Weather data loaded successfully");
    return true;
}

void initializeWeatherData() {
    const char* filePath = "/weather.json";

    if (!LittleFS.exists(filePath)) {
        Serial.printf("File %s does not exist. Creating a default file.\n", filePath);

        WeatherData defaultWeather;
        defaultWeather.temperature = "N/A";
        defaultWeather.condition = "Unknown";
        defaultWeather.icon = "unknown";
        defaultWeather.sunrise = "N/A";
        defaultWeather.sunset = "N/A";
        defaultWeather.wind_speed = "0";
        defaultWeather.humidity = "0";
        defaultWeather.lastUpdate = 0;
        defaultWeather.id = 666;

                // Use system time if available, fallback to 0 if not
        time_t currentTime = time(nullptr);
        if (currentTime == -1) {
            Serial.println("Failed to get system time. Defaulting to epoch.");
            defaultWeather.dt = 0;  // Fallback to epoch
        } else {
            defaultWeather.dt = static_cast<unsigned long>(currentTime);
            Serial.printf("Default weather timestamp set to: %lu\n", defaultWeather.dt);
        }

        saveWeatherDataToFile(filePath, defaultWeather);
        printCurrentWeather();
    }
    updateWeatherData();
}


void updateWeatherData() {
    // Get the current time from the system
    time_t currentTime = time(nullptr);

    if (currentTime == -1) {
        Serial.println("Failed to get system time. Skipping weather update.");
        return;  // Exit if the system time is not available
    }

    // Load weather data from file
    if (!loadWeatherDataFromFile("/weather.json", currentWeatherData)) {
        Serial.println("Failed to load weather data. Initializing defaults.");
        currentWeatherData.dt = 0;  // Force fetch on first run
    }

    // Print current and last update times for debugging
    Serial.printf("Current time (UNIX): %ld\n", currentTime);
    Serial.printf("Weather data timestamp (UNIX): %ld\n", currentWeatherData.dt);
    Serial.printf("Time since last update (seconds): %ld\n", currentTime - currentWeatherData.dt);

    // Check if the weather data needs to be updated
    if (currentWeatherData.dt == 0 || (currentTime - currentWeatherData.dt) >= 360) {
        Serial.println("Fetching new weather data...");

        // Fetch new weather data
        printCurrentWeather();

        // Update `dt` with the current system time
        //currentWeatherData.dt = static_cast<unsigned long>(currentTime);

        // Save updated weather data to file
        saveWeatherDataToFile("/weather.json", currentWeatherData); 
    }
   
        Serial.println("Weather data is up-to-date. Skipping fetch.");
    

    // Update the UI with current weather data
    lv_label_set_text(ui_WeatherLabel, currentWeatherData.temperature.c_str());

    // Get the file path for the icon
    const char* iconPath = getMeteoconIcon(currentWeatherData.id, true);
    Serial.printf("Icon path: %s\n", iconPath);

    // Update the weather icon on the UI
    lv_img_set_src(ui_WeatherImage, iconPath);
    lv_color_t sci_fi_blue = lv_color_make(0, 200, 255); // Cyan blue color
    lv_obj_set_style_img_recolor(ui_WeatherImage, sci_fi_blue, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(ui_WeatherImage, LV_OPA_90, LV_PART_MAIN);
}

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}




const char* getMeteoconIcon(uint16_t id, bool today)
{
    if (today && id / 100 == 8 && (currentWeatherData.sunrise < currentWeatherData.sunset)) id += 1000; 

    if (id / 100 == 2) return "A:/lvgl/icons/thunderstorm.png";
    if (id / 100 == 3) return "A:/lvgl/icons/drizzle.png";
    if (id / 100 == 4) return "A:/lvgl/icons/unknown.png";
    if (id == 500) return "A:/lvgl/icons/lightRain.png";
    else if (id == 511) return "A:/lvgl/icons/sleet.png";
    else if (id / 100 == 5) return "A:/lvgl/icons/rain.png";
    if (id >= 611 && id <= 616) return "A:/lvgl/icons/sleet.png";
    else if (id / 100 == 6) return "A:/lvgl/icons/snow.png";
    if (id / 100 == 7) return "A:/lvgl/icons/fog.png";
    if (id == 800) return "A:/lvgl/icons/clear-day.png";
    if (id == 801) return "A:/lvgl/icons/partly-cloudy-day.png";
    if (id == 802) return "A:/lvgl/icons/cloudy.png";
    if (id == 803) return "A:/lvgl/icons/cloudy.png";
    if (id == 804) return "A:/lvgl/icons/cloudy.png";
    if (id == 1800) return "A:/lvgl/icons/clear-night.png";
    if (id == 1801) return "A:/lvgl/icons/partly-cloudy-night.png";
    if (id == 1802) return "A:/lvgl/icons/cloudy.png";
    if (id == 1803) return "A:/lvgl/icons/cloudy.png";
    if (id == 1804) return "A:/lvgl/icons/cloudy.png";
    if (id == 666) return "A:/lvgl/icons/unknown.png";
    return "A:/lvgl/icons/unknown.png";
}



void printCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  OW_current *current = new OW_current;
  OW_hourly *hourly = new OW_hourly;
  OW_daily  *daily = new OW_daily;

  Serial.println("\nRequesting weather information from OpenWeather... ");

  //On the ESP8266 (only) the library by default uses BearSSL, another option is to use AXTLS
  //For problems with ESP8266 stability, use AXTLS by adding a false parameter thus       vvvvv
  //ow.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language, false);

  ow.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);
  Serial.println("");
  Serial.println("Weather from Open Weather\n");

  // Position as reported by Open Weather
  Serial.print("Latitude            : "); Serial.println(ow.lat);
  Serial.print("Longitude           : "); Serial.println(ow.lon);
  // We can use the timezone to set the offset eventually...
  Serial.print("Timezone            : "); Serial.println(ow.timezone);
  Serial.println();

  if (current)
  {
    Serial.println("############### Current weather ###############\n");
    Serial.print("dt (time)        : "); Serial.println(strTime(current->dt));
    Serial.print("sunrise          : "); Serial.println(strTime(current->sunrise));
    Serial.print("sunset           : "); Serial.println(strTime(current->sunset));
    Serial.print("temp             : "); Serial.println(current->temp);
    Serial.print("feels_like       : "); Serial.println(current->feels_like);
    Serial.print("pressure         : "); Serial.println(current->pressure);
    Serial.print("humidity         : "); Serial.println(current->humidity);
    Serial.print("dew_point        : "); Serial.println(current->dew_point);
    Serial.print("uvi              : "); Serial.println(current->uvi);
    Serial.print("clouds           : "); Serial.println(current->clouds);
    Serial.print("visibility       : "); Serial.println(current->visibility);
    Serial.print("wind_speed       : "); Serial.println(current->wind_speed);
    Serial.print("wind_gust        : "); Serial.println(current->wind_gust);
    Serial.print("wind_deg         : "); Serial.println(current->wind_deg);
    Serial.print("rain             : "); Serial.println(current->rain);
    Serial.print("snow             : "); Serial.println(current->snow);
    Serial.println();
    Serial.print("id               : "); Serial.println(current->id);
    Serial.print("main             : "); Serial.println(current->main);
    Serial.print("description      : "); Serial.println(current->description);
    Serial.print("icon             : "); Serial.println(current->icon);

    Serial.println();

    currentWeatherData.temperature = String(current->temp, 1) + "°C";
        currentWeatherData.condition = current->description;
        currentWeatherData.icon = current->icon;
        currentWeatherData.sunrise = strTime(current->sunrise);
        currentWeatherData.sunset = strTime(current->sunset);
        currentWeatherData.wind_speed = String(current->wind_speed, 1) + " m/s";
        currentWeatherData.humidity = String(current->humidity) + "%";
        currentWeatherData.id = uint16_t (current->id);
        currentWeatherData.dt = (current->dt);


    // Setting the UI now.

/*             // Update the WeatherData structure
        currentWeatherData.temperature = String(current->temp, 1) + "°C";
        currentWeatherData.condition = current->description;
        currentWeatherData.icon = current->icon;
        currentWeatherData.sunrise = strTime(current->sunrise);
        currentWeatherData.sunset = strTime(current->sunset);
        currentWeatherData.wind_speed = String(current->wind_speed, 1) + " m/s";
        currentWeatherData.humidity = String(current->humidity) + "%";

      lv_label_set_text(ui_WeatherLabel, currentWeatherData.temperature.c_str());
      //lv_label_set_text(ui_ConditionLabel, currentWeatherData.condition.c_str());

        // Get the file path for the icon
        const char* iconPath = getMeteoconIcon(current->id, true);
        Serial.println("Icon path set:");
        Serial.println(iconPath);

        // Update the image source dynamically
        lv_img_set_src(ui_WeatherImage, iconPath);
         lv_color_t sci_fi_blue = lv_color_make(0, 200, 255); // Cyan blue color
        lv_obj_set_style_img_recolor(ui_WeatherImage, sci_fi_blue, LV_PART_MAIN);
        lv_obj_set_style_img_recolor_opa(ui_WeatherImage, LV_OPA_90, LV_PART_MAIN); */

  }

  if (hourly)
  {
    Serial.println("############### Hourly weather  ###############\n");
    for (int i = 0; i < MAX_HOURS; i++)
    {
      Serial.print("Hourly summary  "); if (i < 10) Serial.print(" "); Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : "); Serial.println(strTime(hourly->dt[i]));
      Serial.print("temp             : "); Serial.println(hourly->temp[i]);
      Serial.print("feels_like       : "); Serial.println(hourly->feels_like[i]);
      Serial.print("pressure         : "); Serial.println(hourly->pressure[i]);
      Serial.print("humidity         : "); Serial.println(hourly->humidity[i]);
      Serial.print("dew_point        : "); Serial.println(hourly->dew_point[i]);
      Serial.print("clouds           : "); Serial.println(hourly->clouds[i]);
      Serial.print("wind_speed       : "); Serial.println(hourly->wind_speed[i]);
      Serial.print("wind_gust        : "); Serial.println(hourly->wind_gust[i]);
      Serial.print("wind_deg         : "); Serial.println(hourly->wind_deg[i]);
      Serial.print("rain             : "); Serial.println(hourly->rain[i]);
      Serial.print("snow             : "); Serial.println(hourly->snow[i]);
      Serial.println();
      Serial.print("id               : "); Serial.println(hourly->id[i]);
      Serial.print("main             : "); Serial.println(hourly->main[i]);
      Serial.print("description      : "); Serial.println(hourly->description[i]);
      Serial.print("icon             : "); Serial.println(hourly->icon[i]);
      Serial.print("pop              : "); Serial.println(hourly->pop[i]);

      Serial.println();
    }
  }

  if (daily)
  {
    Serial.println("###############  Daily weather  ###############\n");
    for (int i = 0; i < MAX_DAYS; i++)
    {
      Serial.print("Daily summary   "); if (i < 10) Serial.print(" "); Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : "); Serial.println(strTime(daily->dt[i]));
      Serial.print("sunrise          : "); Serial.println(strTime(daily->sunrise[i]));
      Serial.print("sunset           : "); Serial.println(strTime(daily->sunset[i]));

      Serial.print("temp.morn        : "); Serial.println(daily->temp_morn[i]);
      Serial.print("temp.day         : "); Serial.println(daily->temp_day[i]);
      Serial.print("temp.eve         : "); Serial.println(daily->temp_eve[i]);
      Serial.print("temp.night       : "); Serial.println(daily->temp_night[i]);
      Serial.print("temp.min         : "); Serial.println(daily->temp_min[i]);
      Serial.print("temp.max         : "); Serial.println(daily->temp_max[i]);

      Serial.print("feels_like.morn  : "); Serial.println(daily->feels_like_morn[i]);
      Serial.print("feels_like.day   : "); Serial.println(daily->feels_like_day[i]);
      Serial.print("feels_like.eve   : "); Serial.println(daily->feels_like_eve[i]);
      Serial.print("feels_like.night : "); Serial.println(daily->feels_like_night[i]);

      Serial.print("pressure         : "); Serial.println(daily->pressure[i]);
      Serial.print("humidity         : "); Serial.println(daily->humidity[i]);
      Serial.print("dew_point        : "); Serial.println(daily->dew_point[i]);
      Serial.print("uvi              : "); Serial.println(daily->uvi[i]);
      Serial.print("clouds           : "); Serial.println(daily->clouds[i]);
      Serial.print("visibility       : "); Serial.println(daily->visibility[i]);
      Serial.print("wind_speed       : "); Serial.println(daily->wind_speed[i]);
      Serial.print("wind_gust        : "); Serial.println(daily->wind_gust[i]);
      Serial.print("wind_deg         : "); Serial.println(daily->wind_deg[i]);
      Serial.print("rain             : "); Serial.println(daily->rain[i]);
      Serial.print("snow             : "); Serial.println(daily->snow[i]);
      Serial.println();
      Serial.print("id               : "); Serial.println(daily->id[i]);
      Serial.print("main             : "); Serial.println(daily->main[i]);
      Serial.print("description      : "); Serial.println(daily->description[i]);
      Serial.print("icon             : "); Serial.println(daily->icon[i]);
      Serial.print("pop              : "); Serial.println(daily->pop[i]);

      Serial.println();
    }
  }

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete hourly;
  delete daily;
}

