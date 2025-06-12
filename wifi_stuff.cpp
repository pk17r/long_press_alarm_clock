#include <string.h>
#include <string>
#include <cstddef>
#include "wifi_stuff.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "nvs_preferences.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "rtc.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// Web OTA Update https://github.com/programmer131/ESP8266_ESP32_SelfUpdate/tree/master
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>


WiFiStuff::WiFiStuff() {

  nvs_preferences->RetrieveWiFiDetails(wifi_ssid_, wifi_password_);

  nvs_preferences->RetrieveLocationDetails(location_zip_code_, location_country_code_, city_);
  weather_units_metric_not_imperial_ = nvs_preferences->RetrieveWeatherUnits();

  TurnWiFiOff();

  PrintLn(__func__, kInitializedStr);
}

void WiFiStuff::SaveWiFiDetails() {
  nvs_preferences->SaveWiFiDetails(wifi_ssid_, wifi_password_);
  could_not_connect_to_wifi_ = false;
}

std::string WiFiStuff::WiFiDetailsShortString() {
  //std::string wifi_ssid_passwd_value = wifi_ssid_.substr(0,6) + "*, ";
  //int i = 0;
  //while(i <= 5) {
  //  if(i > wifi_password_.size() - 1) break;
  //  if(i%2 == 0)
  //    wifi_ssid_passwd_value += wifi_password_[i];
  //  else
  //    wifi_ssid_passwd_value += "*";
  //  i++;
  //}
  //return wifi_ssid_passwd_value;
  return wifi_ssid_.substr(0,16);
}

void WiFiStuff::SaveNewLocationAndClearCity() {
  city_ = "";
  nvs_preferences->SaveLocationDetails(location_zip_code_, location_country_code_, city_);
  incorrect_zip_code = false;
  got_weather_info_ = false;
}

void WiFiStuff::SaveNewWeatherUnits() {
  nvs_preferences->SaveWeatherUnits(weather_units_metric_not_imperial_);
  got_weather_info_ = false;
}

bool WiFiStuff::TurnWiFiOn() {

  PrintLn(__func__);
  mins_from_last_wifi_connect_try_ = 0;
  WiFi.mode(WIFI_STA);
  delay(1);
  WiFi.persistent(true);
  delay(1);
  #ifdef MORE_LOGS
  PrintLn(wifi_ssid_.c_str());
  PrintLn(wifi_password_.c_str());
  #endif
  WiFi.begin(wifi_ssid_.c_str(), wifi_password_.c_str());
  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    PrintLn(".");
    i++;
    if(i >= 5) break;
  }
  if(WiFi.status() == WL_CONNECTED) {
    #ifdef MORE_LOGS
    PrintLn("WiFiStuff::TurnWiFiOn(): WiFi Connected.");
    #endif
    if(0x01 == My_Hw_Version)
      digitalWrite(WIFI_LED(), HIGH);
    wifi_connected_ = true;
    could_not_connect_to_wifi_ = false;
  }
  else {
    #ifdef MORE_LOGS
    PrintLn("WiFiStuff::TurnWiFiOn(): Could NOT connect to WiFi.");
    #endif
    if(0x01 == My_Hw_Version)
      digitalWrite(WIFI_LED(), LOW);
    wifi_connected_ = false;
    could_not_connect_to_wifi_ = true;
  }

  return wifi_connected_;
}

void WiFiStuff::TurnWiFiOff() {
  PrintLn(__func__);
  WiFi.persistent(false);
  delay(1);
  WiFi.mode(WIFI_OFF);
  delay(1);
  WiFi.disconnect();
  if(0x01 == My_Hw_Version)
    digitalWrite(WIFI_LED(), LOW);
  wifi_connected_ = false;
}

bool WiFiStuff::GetTodaysWeatherInfo() {
  got_weather_info_ = false;

  std::string func_name = __func__;

  // no point fetching weather info if openWeatherMapApiKey is empty
  if(openWeatherMapApiKey.size() == 0) {
    PrintLn(func_name, "No Key");
    return false;
  }

  // don't fetch frequently otherwise can get banned
  if(last_fetch_weather_info_time_ms_ != 0 && millis() - last_fetch_weather_info_time_ms_ < kFetchWeatherInfoMinIntervalMs) {
    get_weather_info_wait_seconds_ = (kFetchWeatherInfoMinIntervalMs - (millis() - last_fetch_weather_info_time_ms_)) / 1000;
    #ifdef MORE_LOGS
    PrintLn(func_name, "Wait more");
    #endif
    return false;
  }
  get_weather_info_wait_seconds_ = 0;

  // turn On Wifi
  if(!wifi_connected_) {
    if(!TurnWiFiOn()) {
      #ifdef MORE_LOGS
      PrintLn(func_name, "No WiFi");
      #endif
      return false;
    }
  }

  // Your Domain name with URL path or IP address with path
  // std::string openWeatherMapApiKey = 

  //https://api.openweathermap.org/data/2.5/weather?zip=92104,US&appid=
  //{"coord":{"lon":-117.1272,"lat":32.7454},"weather":[{"id":701,"main":"Mist","description":"mist","icon":"50n"}],"base":"stations","main":{"temp":284.81,"feels_like":284.41,"temp_min":283.18,"temp_max":286.57,"pressure":1020,"humidity":91},"visibility":10000,"wind":{"speed":3.09,"deg":0},"clouds":{"all":75},"dt":1708677188,"sys":{"type":2,"id":2019527,"country":"US","sunrise":1708698233,"sunset":1708738818},"timezone":-28800,"id":0,"name":"San Diego","cod":200}

  // Replace with your country code and city
  // std::string city = "San%20Diego";
  // std::string countryCode = "840";

  // Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED) {
    // std::string serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city_copy + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=imperial";
    std::string serverPath = "http://api.openweathermap.org/data/2.5/weather?zip=" + location_zip_code_ + "," + location_country_code_ + "&appid=" + openWeatherMapApiKey + "&units=" + (weather_units_metric_not_imperial_ ? "metric" : "imperial" );
    WiFiClient client;
    HTTPClient http;
    #ifdef MORE_LOGS
    PrintLn(serverPath);
    #endif

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverPath.c_str());

    // Send HTTP POST request
    int httpResponseCode = http.GET();
    last_fetch_weather_info_time_ms_ = millis();

    String jsonBuffer = "{}"; 

    if (httpResponseCode>0) {
      jsonBuffer = http.getString();
    }
    // Free resources
    http.end();

    PrintLn(jsonBuffer.c_str());
    JSONVar myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    // if (JSON.typeof(myObject) == "undefined" || httpResponseCode <= 0) {
    //   Serial.println("Parsing input failed!");
    // }
    if(httpResponseCode >= 200 && httpResponseCode < 300)
    {
      // got response
      got_weather_info_ = true;

      #ifdef MORE_LOGS
        Serial.print("JSON object = ");
        Serial.println(myObject);
      #endif
      weather_main_.assign(myObject["weather"][0]["main"]);
      weather_description_.assign(myObject["weather"][0]["description"]);
      double val = atof(JSONVar::stringify(myObject["main"]["temp"]).c_str());
      char valArr[10]; sprintf(valArr,"%.1f%c", val, (weather_units_metric_not_imperial_ ? 'C' : 'F'));
      weather_temp_.assign(valArr);
      val = atof(JSONVar::stringify(myObject["main"]["feels_like"]).c_str());
      sprintf(valArr,"%.1f%c", val, (weather_units_metric_not_imperial_ ? 'C' : 'F'));
      weather_temp_feels_like_.assign(valArr);
      val = atof(JSONVar::stringify(myObject["main"]["temp_max"]).c_str());
      sprintf(valArr,"%.1f%c", val, (weather_units_metric_not_imperial_ ? 'C' : 'F'));
      weather_temp_max_.assign(valArr);
      val = atof(JSONVar::stringify(myObject["main"]["temp_min"]).c_str());
      sprintf(valArr,"%.1f%c", val, (weather_units_metric_not_imperial_ ? 'C' : 'F'));
      weather_temp_min_.assign(valArr);
      val = atof(JSONVar::stringify(myObject["wind"]["speed"]).c_str());
      sprintf(valArr,"%d%s", (int)val, (weather_units_metric_not_imperial_ ? "m/s" : "mi/hr"));
      weather_wind_speed_.assign(valArr);
      weather_humidity_.assign(JSONVar::stringify(myObject["main"]["humidity"]).c_str());
      weather_humidity_ = weather_humidity_ + '%';
      city_.assign(myObject["name"]);
      nvs_preferences->SaveCityName(city_);
      gmt_offset_sec_ = atoi(JSONVar::stringify(myObject["timezone"]).c_str());
      #ifdef MORE_LOGS
        PrintLn("weather_main ", weather_main_.c_str());
        PrintLn("weather_description ", weather_description_.c_str());
        PrintLn("weather_temp ", weather_temp_.c_str());
        PrintLn("weather_temp_feels_like_ ", weather_temp_feels_like_.c_str());
        PrintLn("weather_temp_max ", weather_temp_max_.c_str());
        PrintLn("weather_temp_min ", weather_temp_min_.c_str());
        PrintLn("weather_wind_speed ", weather_wind_speed_.c_str());
        PrintLn("weather_humidity ", weather_humidity_.c_str());
        PrintLn("city_ ", city_.c_str());
        PrintLn("gmt_offset_sec_ ", gmt_offset_sec_);
      #endif
      weather_fetch_error_message = "";
    }
    else {
      if(httpResponseCode >= 400)
        incorrect_zip_code = true;
      weather_fetch_error_message = JSONVar::stringify(myObject["cod"]).c_str();
      weather_fetch_error_message += kCharSpace;
      weather_fetch_error_message += JSONVar::stringify(myObject["message"]).c_str();
      weather_fetch_error_message.erase(std::remove(weather_fetch_error_message.begin(), weather_fetch_error_message.end(), '"'), weather_fetch_error_message.end());
    }
  }
  else {
    #ifdef MORE_LOGS
    PrintLn(func_name, "No WiFi");
    #endif
  }

  // turn off WiFi
  // TurnWiFiOff();
  return true;
}

bool WiFiStuff::GetTimeFromNtpServer() {
  manual_time_update_successful_ = false;

  if(!got_weather_info_) { // we need gmt_offset_sec_ before getting time update!
    GetTodaysWeatherInfo();
    PrintLn(__func__, got_weather_info_);
    if(!got_weather_info_) {
      return false;
    }
  }

  // turn On Wifi
  if(!wifi_connected_) {
    if(!TurnWiFiOn()) {
      return false;
    }
  }

  bool returnVal = false;

  // Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED) {

    const char* NTP_SERVER = "pool.ntp.org";
    // const long  GMT_OFFSET_SEC = -8*60*60;

    // Define an NTP Client object
    WiFiUDP udpSocket;
    NTPClient ntpClient(udpSocket, NTP_SERVER, gmt_offset_sec_);

    ntpClient.begin();
    returnVal = ntpClient.update();

    if(returnVal) {
      unsigned long epoch_since_1970 = ntpClient.getEpochTime();
      int hours = ntpClient.getHours();
      int minutes = ntpClient.getMinutes();
      int seconds = ntpClient.getSeconds();
      int dayOfWeekSunday0 = ntpClient.getDay();

      PrintLn("NTP Time:",(std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds) + " gmt_offset_sec=" + std::to_string(gmt_offset_sec_) + " " + std::to_string(dayOfWeekSunday0) + kDaysTable_[dayOfWeekSunday0] + " epoch=" + std::to_string(epoch_since_1970)));

      int today, month, year;
      ConvertEpochIntoDate(epoch_since_1970, today, month, year);

      // RTC::SetRtcTimeAndDate(uint8_t second, uint8_t minute, uint8_t hour_24_hr_mode, uint8_t dayOfWeek_Sun_is_1, uint8_t day, uint8_t month_Jan_is_1, uint16_t year)
      returnVal = rtc->SetRtcTimeAndDate(seconds, minutes, hours, dayOfWeekSunday0 + 1, today, month, year);

    }

    ntpClient.end();

  }
  else {
    #ifdef MORE_LOGS
    PrintLn(__func__, "No WiFi");
    #endif
  }

  // // test
  // Serial.println();
  // Serial.print("Test Date:  8/20/2024   "); ConvertEpochIntoDate(1724195000);
  // Serial.print("Test Date:  10/20/2024   "); ConvertEpochIntoDate(1729385200);
  // Serial.print("Test Date:  10/10/2029   "); ConvertEpochIntoDate(1886367800);
  // Serial.print("Test Date:  3/1/2036   "); ConvertEpochIntoDate(2087942600);
  // Serial.print("Test Date:  12/31/2024   "); ConvertEpochIntoDate(1735603400);
  // Serial.print("Test Date:  1/1/2025   "); ConvertEpochIntoDate(1735689800);
  // Serial.print("Test Date:  1/31/2025   "); ConvertEpochIntoDate(1738281800);
  // Serial.print("Test Date:  3/1/2028   "); ConvertEpochIntoDate(1835481800);

  // turn off WiFi
  // TurnWiFiOff();

  manual_time_update_successful_ = returnVal;
  PrintLn(__func__, returnVal);
  return returnVal;
}

void WiFiStuff::ConvertEpochIntoDate(unsigned long epoch_since_1970, int &today, int &month, int &year) {

  unsigned long epoch_Jan_1_2023_12_AM = 1704067200;
  float day = static_cast<float>(epoch_since_1970 - epoch_Jan_1_2023_12_AM) / (24*60*60);
  year = 2024;
  int monthJan0 = 0;
  // calculate year
  while(1) {
    if(day - 365 - (year % 4 == 0 ? 1 : 0) < 0)
      break;
    day -= 365 + (year % 4 == 0 ? 1 : 0);
    year++;
  }
  // calculate month
  // jan
  if(day - 31 > 0) {
    monthJan0++; day -= 31;
    // feb
    if(day - 28 - (year % 4 == 0 ? 1 : 0) > 0) {
      monthJan0++; day -= 28 + (year % 4 == 0 ? 1 : 0);
      // march
      if(day - 31 > 0) {
        monthJan0++; day -= 31;
        // apr
        if(day - 30 > 0) {
          monthJan0++; day -= 30;
          // may
          if(day - 31 > 0) {
            monthJan0++; day -= 31;
            // jun
            if(day - 30 > 0) {
              monthJan0++; day -= 30;
              // jul
              if(day - 31 > 0) {
                monthJan0++; day -= 31;
                // aug
                if(day - 31 > 0) {
                  monthJan0++; day -= 31;
                  // sep
                  if(day - 30 > 0) {
                    monthJan0++; day -= 30;
                    // oct
                    if(day - 31 > 0) {
                      monthJan0++; day -= 31;
                      // nov
                      if(day - 30 > 0) {
                        monthJan0++; day -= 30;
                        // dec
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  today = ceil(day);
  PrintLn(__func__, (std::to_string(epoch_since_1970) + " = " + std::to_string(monthJan0) + kMonthsTable[monthJan0] + " " + std::to_string(today) + " " + std::to_string(year)));
  month = monthJan0 + 1;
}

void WiFiStuff::StartSetWiFiSoftAP() {
  extern void _SoftAPWiFiDetails();

  extern AsyncWebServer* server;

  TurnWiFiOff();
  delay(100);

  if(server != NULL) {
    delete server;
    server = NULL;
  }

  WiFi.mode(WIFI_AP);
  delay(100);
  got_SAP_user_input_ = false;
  save_SAP_details_ = false;

  server = new AsyncWebServer(80);

  // Connect to Wi-Fi network with SSID and password
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(softApSsid);

  IPAddress IP = WiFi.softAPIP();
  soft_AP_IP = IP.toString().c_str();
  PrintLn(__func__, soft_AP_IP);

  server->begin();
  if(0x01 == My_Hw_Version)
    digitalWrite((WIFI_LED()), HIGH);

  _SoftAPWiFiDetails();
}

void WiFiStuff::StopSetWiFiSoftAP() {
  extern AsyncWebServer* server;
  extern String temp_ssid_str, temp_passwd_str;

  // To access your stored values on ssid_str, passwd_str
  PrintLn(__func__, temp_ssid_str.c_str());
  PrintLn(__func__, temp_passwd_str.c_str());

  TurnWiFiOff();
  delay(100);

  if(server != NULL) {
    server->end();

    delete server;
    server = NULL;
  }

  if(save_SAP_details_) {
    //wifi_ssid_ = temp_ssid_str.c_str();
    wifi_password_ = temp_passwd_str.c_str();
    SaveWiFiDetails();
  }

  got_SAP_user_input_ = false;
  save_SAP_details_ = false;
}

void WiFiStuff::StartSetLocationLocalServer() {
  extern void _LocalServerLocationInputs();

  extern AsyncWebServer* server;

  TurnWiFiOff();
  delay(100);

  if(server != NULL) {
    delete server;
    server = NULL;
  }

  soft_AP_IP = "";
  got_SAP_user_input_ = false;
  save_SAP_details_ = false;

  if(!TurnWiFiOn())
    return;
  delay(100);

  server = new AsyncWebServer(80);

  IPAddress IP = WiFi.localIP();
  soft_AP_IP = IP.toString().c_str();
  PrintLn(__func__, soft_AP_IP);
  
  _LocalServerLocationInputs();
}

void WiFiStuff::StopSetLocationLocalServer() {
  extern AsyncWebServer* server;
  extern String temp_owner_name_str, temp_zip_pin_str, temp_country_code_str;

  // To access your stored values on ssid_str, passwd_str
  PrintLn(__func__, temp_zip_pin_str.c_str());
  PrintLn(__func__, temp_country_code_str.c_str());

  TurnWiFiOff();
  delay(100);

  if(server != NULL) {
    server->end();

    delete server;
    server = NULL;
  }

  if(save_SAP_details_) {
    std::string owner_name = temp_owner_name_str.c_str();
    nvs_preferences->SaveOwnerName(owner_name);
    location_zip_code_ = temp_zip_pin_str.c_str();
    location_country_code_ = temp_country_code_str.c_str();
    SaveNewLocationAndClearCity();
  }

  got_SAP_user_input_ = false;
  save_SAP_details_ = false;
}

AsyncWebServer* server = NULL;

const char* kHtmlParamKeySsid = "html_ssid";
const char* kHtmlParamKeyPasswd = "html_passwd";
const char* kHtmlParamKeyOwnerName = "html_owner_name";
const char* kHtmlParamKeyZipPin = "html_zip_pin";
const char* kHtmlParamKeyCountryCode = "html_country_code";

String temp_ssid_str = "Enter SSID";
String temp_passwd_str = "Enter Passwd";
String temp_owner_name_str = "Enter Owner Name";
String temp_zip_pin_str = "Enter ZIP/PIN";
String temp_country_code_str = "Enter Country Code";

// HTML web page to handle 2 input fields (html_ssid, html_passwd)
const char index_html_wifi_details[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Long Press Alarm Clock</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      setTimeout(function() { alert('sent!'); }, 1);
    }
  </script></head><body>
  <form action="/get" target="hidden-form">
  	<a href="https://github.com/pk17r/Long_Press_Alarm_Clock/tree/release" target="_blank"><h3>Long Press Alarm Clock</h3></a>
    <h4>Enter WiFi Password for %html_ssid%</h4>
    <input type="text" name="html_passwd" value="%html_passwd%"><br><br>
    <input type="submit" value="Submit" onclick="submitMessage()"><br>
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

// HTML web page to handle 2 input fields (html_zip_pin, html_country_code)
const char index_html_location_details[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Long Press Alarm Clock</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      setTimeout(function() { alert('sent!'); }, 1);
    }
  </script></head><body>
  <form action="/get" target="hidden-form">
  	<a href="https://github.com/pk17r/Long_Press_Alarm_Clock/tree/release" target="_blank"><h3>Long Press Alarm Clock</h3></a>
    <label>Owner Name:</label><br>
    <input type="text" name="html_owner_name" value="%html_owner_name%"><br><br>
    <h4>Enter Location Details:</h4>
    <label>Location ZIP/PIN Code:</label><br>
    <input type="number" name="html_zip_pin" value="%html_zip_pin%"><br><br>
    <label>2-Letter Country Code (</label>
    <a href="https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes#Current_ISO_3166_country_codes" target="_blank">List</a>
    <label>):</label><br>
    <input type="text" name="html_country_code" value="%html_country_code%" oninput="this.value = this.value.toUpperCase()"><br><br>
    <input type="submit" value="Submit" onclick="submitMessage()"><br>
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

// Replaces placeholder with stored values
String processor(const String& var){
  if(strcmp(var.c_str(), kHtmlParamKeySsid) == 0){
    return temp_ssid_str;
  }
  else if(strcmp(var.c_str(), kHtmlParamKeyPasswd) == 0){
    return temp_passwd_str;
  }
  else if(strcmp(var.c_str(), kHtmlParamKeyOwnerName) == 0){
    return temp_owner_name_str;
  }
  else if(strcmp(var.c_str(), kHtmlParamKeyZipPin) == 0){
    return temp_zip_pin_str;
  }
  else if(strcmp(var.c_str(), kHtmlParamKeyCountryCode) == 0){
    return temp_country_code_str;
  }
  return String();
}

void _SoftAPWiFiDetails() {

  temp_ssid_str = wifi_stuff->wifi_ssid_.c_str();
  temp_passwd_str = "-here-"; // wifi_stuff->wifi_password_.c_str();

  extern String processor(const String& var);

  // Send web page with input fields to client
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html_wifi_details, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server->on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET html_passwd value on <ESP_IP>/get?html_passwd=<inputMessage>
    if (request->hasParam(kHtmlParamKeyPasswd)) {
      inputMessage = request->getParam(kHtmlParamKeyPasswd)->value();
      temp_passwd_str = inputMessage;
    }
    PrintLn(__func__, inputMessage.c_str());
    request->send(200, "text/text", inputMessage);
    wifi_stuff->got_SAP_user_input_ = true;
  });

  // server->onNotFound(notFound);
  server->begin();
}

void _LocalServerLocationInputs() {

  std::string owner_name;
  nvs_preferences->RetrieveOwnerName(owner_name);
  temp_owner_name_str = owner_name.c_str();
  temp_zip_pin_str = wifi_stuff->location_zip_code_.c_str();
  temp_country_code_str = wifi_stuff->location_country_code_.c_str();

  extern String processor(const String& var);

  // Send web page with input fields to client
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html_location_details, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server->on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET html_owner_name value on <ESP_IP>/get?html_owner_name=<inputMessage>
    if (request->hasParam(kHtmlParamKeyOwnerName)) {
      inputMessage = request->getParam(kHtmlParamKeyOwnerName)->value();
      temp_owner_name_str = inputMessage;
    }
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(kHtmlParamKeyZipPin)) {
      inputMessage = request->getParam(kHtmlParamKeyZipPin)->value();
      temp_zip_pin_str = inputMessage;
    }
    // GET html_passwd value on <ESP_IP>/get?html_passwd=<inputMessage>
    if (request->hasParam(kHtmlParamKeyCountryCode)) {
      inputMessage = request->getParam(kHtmlParamKeyCountryCode)->value();
      temp_country_code_str = inputMessage;
    }
    PrintLn(__func__, inputMessage.c_str());
    request->send(200, "text/text", inputMessage);
    wifi_stuff->got_SAP_user_input_ = true;
  });

  // server->onNotFound(notFound);
  server->begin();
}

// ESP32 Web OTA Update

// check for available firmware update
// Web OTA Update https://github.com/programmer131/ESP8266_ESP32_SelfUpdate/tree/master
// ESP32 WiFiClientSecure examples: WiFiClientInsecure.ino WiFiClientSecure.ino
bool WiFiStuff::FirmwareVersionCheck() {
  // turn On Wifi
  if(!wifi_connected_)
    if(!TurnWiFiOn())
      return false;

  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += (debug_mode ? URL_fw_Version_debug_mode.c_str() : URL_fw_Version_release.c_str());
  fwurl += "?";
  fwurl += String(rand());
  PrintLn(__func__, fwurl.c_str());
  WiFiClientSecure * client = new WiFiClientSecure;

  if(client)
  {
    if(use_secure_connection)
      client->setCACert(rootCACertificate);
    else
      client->setInsecure();//skip verification

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      PrintLn(__func__, httpCode);
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
        payload = https.getString(); // save received version
      // else
      //   PrintLn("error in downloading version file:", httpCode);
      https.end();
    }
    delete client;
  }

  std::string payload_str = payload.c_str();
  // PrintLn("payload_str: ", payload_str);
  // PrintLn("kFwSearchStr: ", kFwSearchStr);

  int search_str_index = payload_str.find(kFwSearchStr);
  // PrintLn("search_str_index = ", search_str_index);

  if(search_str_index >= 0) {
    int fw_start_index = payload_str.find('"', search_str_index) + 1;
    int fw_end_index = payload_str.find('"', fw_start_index);
    // PrintLn("fw_start_index = ", fw_start_index);
    // PrintLn("fw_end_index = ", fw_end_index);
    std::string fw_str = payload_str.substr(fw_start_index, fw_end_index - fw_start_index);
    PrintLn(__func__, fw_str);
    // PrintLn("Active kFirmwareVersion:", kFirmwareVersion);
    firmware_update_available_str_ = fw_str;

    if(strcmp(fw_str.c_str(), kFirmwareVersion.c_str()) != 0) {
      // PrintLn("New firmware detected");
      firmware_update_available_ = true;
      return true;
    }
    else {
      // PrintLn("Device already on latest firmware version");
      return false;
    }
  }
  return false;
}

// update firmware
// Web OTA Update https://github.com/programmer131/ESP8266_ESP32_SelfUpdate/tree/master
// ESP32 WiFiClientSecure examples: WiFiClientInsecure.ino WiFiClientSecure.ino
void WiFiStuff::UpdateFirmware() {
  // turn On Wifi
  if(!wifi_connected_)
    if(!TurnWiFiOn())
      return;

  WiFiClientSecure client;

  if(use_secure_connection)
    client.setCACert(rootCACertificate);
  else
    client.setInsecure();//skip verification

  httpUpdate.setLedPin(LED_PIN(), HIGH);

  // increase watchdog timeout to 90s to accomodate OTA update
  if(!debug_mode) SetWatchdogTime(kWatchdogTimeoutOtaUpdateMs);

  PrintLn(__func__, (debug_mode ? URL_fw_Bin_debug_mode : URL_fw_Bin_release));
  t_httpUpdate_return ret = httpUpdate.update(client, (debug_mode ? URL_fw_Bin_debug_mode.c_str() : URL_fw_Bin_release.c_str()));

  PrintLn(__func__, ret);
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    PrintLn(__func__, httpUpdate.getLastError());
    PrintLn(__func__, httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    // PrintLn(__func__, "HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    // PrintLn(__func__, "HTTP_UPDATE_OK");
    break;
  }

  PrintLn(__func__, -1);    // if code comes here then UpdateFirmware() was unsuccessful.

  if(!debug_mode) SetWatchdogTime(kWatchdogTimeoutMs);
}

bool WiFiStuff::WiFiScanNetworks() {
  WiFiScanNetworksFreeMemory();
  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  PrintLn(__func__, n);
  if (n == 0)
    return false;
  else
    return true;
  // Delete the scan result to free memory for code below.
  //WiFi.scanDelete();
  //return n;
}

int WiFiStuff::WiFiScanNetworksCount() {
  return WiFi.scanComplete();
}

std::string WiFiStuff::WiFiScanNetworkDetails(int wifi_net_ind) {
  const int ssid_len = 17;
  std::string wifi_network_details(ssid_len, ' ');

  std::string ssid = WiFi.SSID(wifi_net_ind).c_str();
  int rssi = WiFi.RSSI(wifi_net_ind);
  std::string encryptionType = "";
  switch (WiFi.encryptionType(wifi_net_ind)) {
    case WIFI_AUTH_OPEN:
        encryptionType = "Open";
        break;
    case WIFI_AUTH_WEP:
        encryptionType = "WEP";
        break;
    case WIFI_AUTH_WPA_PSK:
        encryptionType = "WPA";
        break;
    case WIFI_AUTH_WPA2_PSK:
        encryptionType = "WPA2";
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        encryptionType = "WPA+2";
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        encryptionType = "WPA2EP";
        break;
    case WIFI_AUTH_WPA3_PSK:
        encryptionType = "WPA3";
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        encryptionType = "WPA2+3";
        break;
    case WIFI_AUTH_WAPI_PSK:
        encryptionType = "WAPI";
        break;
    default:
        encryptionType = "???";
  }

  wifi_network_details.replace(0, std::min(int(ssid.length()), ssid_len), ssid.substr(0,std::min(int(ssid.length()), ssid_len)));
  wifi_network_details += "|" + std::string(String(rssi).c_str()) + " " + encryptionType;
  PrintLn(wifi_network_details);
  return wifi_network_details;
}

std::string WiFiStuff::WiFiScanNetworkSsid(int wifi_net_ind) {
  std::string ssid = WiFi.SSID(wifi_net_ind).c_str();
  PrintLn(__func__, ssid);
  return ssid;
}

void WiFiStuff::WiFiScanNetworksFreeMemory() {
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  // PrintLn("WiFi.scanDelete() called.");
}
