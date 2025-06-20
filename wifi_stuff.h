#include <string>
#ifndef WIFI_STUFF_H
#define WIFI_STUFF_H

#include "common.h"
#include "secrets.h"
#include <sys/_stdint.h>      // try removing it, don't know why it is here

class WiFiStuff {

public:
  WiFiStuff();
  void SaveWiFiDetails();
  std::string WiFiDetailsShortString();
  void SaveNewLocationAndClearCity();
  void SaveNewWeatherUnits();
  bool TurnWiFiOn();
  void TurnWiFiOff();
  bool GetTodaysWeatherInfo();
  bool GetTimeFromNtpServer();
  void StartSetWiFiSoftAP();
  void StopSetWiFiSoftAP();
  void StartSetLocationLocalServer();
  void StopSetLocationLocalServer();
  void UpdateFirmware();
  bool FirmwareVersionCheck();
  bool WiFiScanNetworks();
  int WiFiScanNetworksCount();
  std::string WiFiScanNetworkDetails(int wifi_net_ind);
  std::string WiFiScanNetworkSsid(int wifi_net_ind);
  void WiFiScanNetworksFreeMemory();

  #if defined(MY_WIFI_SSID)   // create a secrets.h file with #define for MY_WIFI_SSID
    std::string wifi_ssid_ = MY_WIFI_SSID;
  #else
    std::string wifi_ssid_ = "";
  #endif
  #if defined(MY_WIFI_PASSWD)   // create a secrets.h file with #define for MY_WIFI_PASSWD
    std::string wifi_password_ = MY_WIFI_PASSWD;
  #else
    std::string wifi_password_ = "";
  #endif
  #if defined(MY_OPEN_WEATHER_MAP_API_KEY)   // create a secrets.h file with #define for MY_OPEN_WEATHER_MAP_API_KEY
    std::string openWeatherMapApiKey = MY_OPEN_WEATHER_MAP_API_KEY;
  #else
    std::string openWeatherMapApiKey = "";
  #endif

  // weather information
  std::string weather_main_ = "";
  std::string weather_description_ = "";
  std::string weather_temp_ = "";
  std::string weather_temp_feels_like_ = "";
  std::string weather_temp_max_ = "";
  std::string weather_temp_min_ = "";
  std::string weather_wind_speed_ = "";
  std::string weather_humidity_ = "";
  std::string city_ = "";
  int32_t gmt_offset_sec_ = 0;

  bool got_weather_info_ = false;   // whether weather information has been pulled
  std::string weather_fetch_error_message = "";
  uint8_t get_weather_info_wait_seconds_ = 0;   // wait to delay weather info pulls
  unsigned long last_fetch_weather_info_time_ms_ = 0;
  const unsigned long kFetchWeatherInfoMinIntervalMs = 5*1000;    //  5 seconds
  bool incorrect_zip_code = false;

  bool manual_time_update_successful_ = false;   // flag used to know if manual time update fetch was success

  std::string location_zip_code_ = "92104";

  std::string location_country_code_ = "US";     // https://developer.accuweather.com/countries-by-region

  bool weather_units_metric_not_imperial_ = false;

  volatile bool wifi_connected_ = false;

  // flag to stop trying auto connect to WiFi
  bool could_not_connect_to_wifi_ = false;
  uint8_t mins_from_last_wifi_connect_try_ = 0;

  std::string soft_AP_IP = "";
  bool got_SAP_user_input_ = false;
  bool save_SAP_details_ = false;

  // flag to to know if new firmware update is available
  bool firmware_update_available_ = false;
  std::string firmware_update_available_str_ = "";

  // https://raw.githubusercontent.com DigiCert root certificate has expiry date of Fri, 15 Jan 2038 12:00:00 GMT
  // download process:
  // Open Mozilla Firefox
  // Go to https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/configuration.h
  // Click Lock Icon on Address Bar -> Connection secure -> More information
  // In newly opened Page Info Window, click Security Tab -> View Certificate
  // In newly opened Certificate for *.github.io tab on Firefox click DigiCert Global Root G2 Tab -> Go to Miscellaneous -> Download PEM(cert)
  // Open github-io.pem in Notepad++ and format it like below
  const char* rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
  "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
  "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
  "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
  "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
  "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
  "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
  "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
  "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
  "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
  "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
  "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
  "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
  "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
  "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
  "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
  "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
  "MrY=\n" \
  "-----END CERTIFICATE-----\n";

  // bool to indicate whether Web OTA Update needs to be secure or insecure
  const bool use_secure_connection = false;

  // Web OTA Update https://github.com/programmer131/ESP8266_ESP32_SelfUpdate/tree/master
  // ESP32 WiFiClientSecure examples: WiFiClientInsecure.ino WiFiClientSecure.ino
  const std::string URL_fw_Version_debug_mode = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/configuration.h";
  const std::string URL_fw_Version_release    = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/release/configuration.h";
  #if defined(MCU_IS_ESP32_S2)
    const std::string URL_fw_Bin_debug_mode = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/build/esp32.esp32.esp32s2/long_press_alarm_clock.ino.bin";
    const std::string URL_fw_Bin_release    = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/release/build/esp32.esp32.esp32s2/long_press_alarm_clock.ino.bin";
  #elif defined(MCU_IS_ESP32_S3)
    const std::string URL_fw_Bin_debug_mode = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/build/esp32.esp32.esp32s3/long_press_alarm_clock.ino.bin";
    const std::string URL_fw_Bin_release    = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/release/build/esp32.esp32.esp32s3/long_press_alarm_clock.ino.bin";
  #elif defined(MCU_IS_ESP32_S2_MINI)
    const std::string URL_fw_Bin_debug_mode = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/build/esp32.esp32.lolin_s2_mini/long_press_alarm_clock.ino.bin";
    const std::string URL_fw_Bin_release    = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/release/build/esp32.esp32.lolin_s2_mini/long_press_alarm_clock.ino.bin";
  #elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)
    const std::string URL_fw_Bin_debug_mode = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/main/build/esp32.esp32.esp32da/long_press_alarm_clock.ino.bin";
    const std::string URL_fw_Bin_release    = "https://raw.githubusercontent.com/pk17r/Long_Press_Alarm_Clock/release/build/esp32.esp32.esp32da/long_press_alarm_clock.ino.bin";
  #endif

// PRIVATE VARIABLES

private:

  void ConvertEpochIntoDate(unsigned long epoch_since_1970, int &today, int &month, int &year);

};

#endif  // WIFI_STUFF_H