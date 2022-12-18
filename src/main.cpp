#include <Arduino.h>
#include "Adafruit_ThinkInk.h"
#include "bitmap.h"
#include <WiFi.h>
#include <WifiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include "icalendar.h"
#include <ctime>
#include "date.h"

#define EPD_DC 7
#define EPD_CS 8
#define EPD_BUSY -1
#define SRAM_CS -1
#define EPD_RESET 6

ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

struct
{
  Event *event = nullptr;
  const unsigned char *icon = nullptr;
  String name = "";
} ClassEvent[5];

void drawCentreString(const char *buf)
{
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((display.width() - w) / 2, (display.height() - h) / 2);
  display.print(buf);
}

void drawCurrentEvent(const char *month, const char *day, const char *uccday, const char *etime)
{
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.setCursor(236, 20);
  display.print(month);
  display.setCursor(236, 38);
  display.print(day);
  display.setCursor(236, 56);
  display.print(uccday);
  display.setCursor(236, 74);
  display.print(etime);
}

void drawTopLines()
{
  // get display width and height with padding of 10px
  int width = display.width() - 10;
  int height = display.height() - 10;

  display.drawBitmap(10, 10, epd_bitmap_short, 46, 5, EPD_BLACK);
  display.drawBitmap(66, 10, epd_bitmap_short, 46, 5, EPD_BLACK);
  display.drawBitmap(122, 10, epd_bitmap_short, 46, 5, EPD_BLACK);
  display.drawBitmap(180, 10, epd_bitmap_short, 46, 5, EPD_BLACK);
  display.drawBitmap(236, 10, epd_bitmap_short, 46, 5, EPD_BLACK);
}

void drawFutureEvent(const int x, const char *name, const char *stime, const char *etime, const unsigned char *icon)
{
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.setCursor(x, 20);
  display.print(name);
  display.setCursor(x, 40);
  display.print(stime);
  display.setCursor(x, 60);
  display.print(etime);
  display.drawBitmap(x, 85, icon, 36, 36, EPD_BLACK);
}

void displayName()
{
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  // set entire display to black
  display.fillRect(0, 0, display.width(), display.height(), EPD_BLACK);
  display.setTextSize(4);
  display.setTextColor(EPD_LIGHT);
  display.setCursor(20, 40);
  drawCentreString("Jefferson D.");
  display.display();
}

void deepSleep()
{
  delay(1500);
  display.powerDown();
  digitalWrite(EPD_RESET, LOW);              // hardware power down mode
  digitalWrite(SPEAKER_SHUTDOWN, LOW);       // off
  digitalWrite(NEOPIXEL_POWER, HIGH);        // off
  esp_sleep_enable_timer_wakeup(6048000000); // 70 days
  esp_deep_sleep_start();
}

void drawSchedule()
{

  // deepSleep();
}

void connectToWiFi()
{
  WiFi.begin("<SSID>", "<PASSWORD>");
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

String getICAL()
{
  HTTPClient http;
  const char *root_ca = "<ROOT_CA>";

  http.begin("<ICAL ADDRESS>", root_ca);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    http.end();
    return payload;
  }
  else
  {
    Serial.println("Error on HTTP request");
    http.end();
  }
};

tuple<String, String> getTime()
{
  timeClient.begin();
  timeClient.setTimeOffset(-18000);
  timeClient.update();
  long epoch = timeClient.getEpochTime();
  // convert epoch to tm struct
  struct tm timeinfo;
  gmtime_r(&epoch, &timeinfo);
  // print date, month and year
  String starttime = String(timeinfo.tm_year + 1900) + String(timeinfo.tm_mon + 1) + String(timeinfo.tm_mday) + "T000000";
  String endtime = String(timeinfo.tm_year + 1900) + String(timeinfo.tm_mon + 1) + String(timeinfo.tm_mday + 1) + "T000000";
  return (make_tuple(starttime, endtime));
}

void getEvents(const char *starttime, const char *endtime)
{
  Event *CurrentEvent;
  String fetchedICAL = getICAL();
  ICalendar Calendar(fetchedICAL.c_str());
  ICalendar::Query SearchQuery(&Calendar);
  SearchQuery.Criteria.From = starttime;
  SearchQuery.Criteria.To = endtime;
  int i = 0;

  while ((CurrentEvent = SearchQuery.GetNextEvent(false)) != NULL)
  {

    string summary = CurrentEvent->Summary;

    if (summary.find("SPANISH") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_spanish;
      ClassEvent[i].name = "SPAN";
      i++;
      continue;
    }

    if (summary.find("ECONOMICS") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_econ;
      ClassEvent[i].name = "ECON";
      i++;
      continue;
    }

    if (summary.find("ENGLISH") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_english;
      ClassEvent[i].name = "ENGL";
      i++;
      continue;
    }

    if (summary.find("MATH") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_math;
      ClassEvent[i].name = "MATH";
      i++;
      continue;
    }

    if (summary.find("PHYSICS") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_physics;
      ClassEvent[i].name = "PHYS";
      i++;
      continue;
    }

    if (summary.find("THEORY") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_tok;
      ClassEvent[i].name = "TOFK";
      i++;
      continue;
    }

    if (summary.find("CHEMISTRY") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_chem;
      ClassEvent[i].name = "CHEM";
      i++;
      continue;
    }

    if (summary.find("Physical") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_activity;
      ClassEvent[i].name = "ACTV";
      i++;
      continue;
    }

    if (summary.find("Seminar") != string::npos)
    {
      ClassEvent[i].event = CurrentEvent;
      ClassEvent[i].icon = epd_bitmap_Icon_awesome_slide;
      ClassEvent[i].name = "SMNR";
      i++;
      continue;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  // start after serial monitor is open
  connectToWiFi();
  String starttime, endtime;
  tie(starttime, endtime) = getTime();
  // tie(starttime, endtime) = make_tuple("20221220T000000", "20221221T000000");
  getEvents(starttime.c_str(), endtime.c_str());
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  // draw top lines
  drawTopLines();
  // drawCurrentEvent("", "10:00", "11:00", "R216", epd_bitmap_math);
  for (int i = 0; i < 5; i++)
  {
    if (ClassEvent[i].name == "")
    {
      drawFutureEvent(10 + 56 * i, "EMPT", "", "", epd_bitmap_none);
      continue;
    }
    // drawFutureEvent(10 + 56 * i, ClassEvent[i].name.c_str(), ClassEvent[i].event->DtStart.Format().c_str(), "1100", epd_bitmap_spanish);
    drawFutureEvent(10 + 56 * i, ClassEvent[i].name.c_str(), ClassEvent[i].event->DtStart.Format().c_str(), ClassEvent[i].event->DtEnd.Format().c_str(), ClassEvent[i].icon);
  }
  display.display();
  deepSleep();
}

void loop()
{
}