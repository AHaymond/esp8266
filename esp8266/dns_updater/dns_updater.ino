// Arduino code for updating a NameCheap DNS record via a URI
// It will blink the builtin LED during operations to provide
// visual feedback since no display or serial out is attached
// to it.
// It is set to run every 15 minutes to make sure that any
// change in a dynamic IP address is corrected quickly.
//
// This was written to work on a knock off Wemos D1 Mini bought
// off of Ebay. The only oddity found is that the voltage constants
// are backwards.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>

const char* ssid = "[SSID]";
const char* password = "[PWD]";
const char* fingerprint = "8A BD E4 A3 CE 85 A3 FB 94 71 B2 05 3D C6 78 7A 4D 55 63 8B";
const char* fingerprint_hc = "B5 C1 94 82 72 37 93 5C 43 FA AD 79 5F 3E F2 8E F0 C3 B8 38";
const int FAST = 1;
const int SLOW = 0;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // initialize onboard LED as output
  digitalWrite(LED_BUILTIN, HIGH); // turn off the LED
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  blink(SLOW,3);
  blink(FAST,10);

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

void loop() {
  String protocol = "https://";
  String subdomain = "[SUBDOMAIN]";
  String domain = "[DOMAIN]";
  String namecheapPass = "[PWD_NAMECHEAP]";
  String url = "/update?host=" + subdomain + "&domain=" + domain + "&password=" + namecheapPass;
  String host = "dynamicdns.park-your-domain.com";
  String uriString = protocol+host+url;


  if((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    http.begin(uriString,fingerprint);

    // Initiate DNS update
    Serial.println("sending DNS update request");
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
        Serial.println("DNS updated");
        blink(SLOW,5);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    // Health Check at www.healthcheck.io
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("Pinging HealthCheck");
      String checkKey = "[CHECK_KEY]"
      String health_url = "https://hchk.io/"+ checkKey;
      // Health Check
      http.begin(health_url, fingerprint_hc);
      http.GET();
      http.end();
      delay(2000);
      blink(FAST,15);
      Serial.println("checked in");
    }
  }
  delay(900000); // Wait 15 minutes

}

// Note:
//
// Discovered voltages are backwards with Wemos D1 Mini
// HIGH == off
// LOW == on
void blink(int speed, int blinks) {
  int rate;
  if (speed == FAST) {
    rate = 50;
  } else if (speed == SLOW){
    rate = 200;
  }

  for(int i = 0; i < blinks; i++) {
    digitalWrite(LED_BUILTIN, LOW); // turn on LED with voltage LOW
    delay(rate);
    digitalWrite(LED_BUILTIN, HIGH);  // turn off LED with voltage HIGH
    delay(rate);
  }
  digitalWrite(LED_BUILTIN, HIGH); // turn off the LED
}


