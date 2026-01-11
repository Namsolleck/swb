#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

WebServer server(80);
Preferences preferences;

String ssid = "";
String password = "";

// Poprawiony HTML z pełną strukturą i lepszymi inputami
const char* htmlAP = 
  "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>input{display:block;margin:10px 0;padding:10px;width:90%;}</style></head>"
  "<body><h2>Konfiguracja WiFi</h2>"
  "<form action='/save' method='POST'>"
  "SSID:<input type='text' name='ssid' placeholder='Nazwa sieci'>"
  "Haslo:<input type='password' name='pass' placeholder='Haslo'>"
  "<input type='submit' value='Polacz i zapisz'>"
  "</form></body></html>";

void setup() {
  Serial.begin(115200);
  delay(1000); // Czas na ustabilizowanie Seriala
  
  preferences.begin("wifi-config", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");

  if (ssid == "" || ssid == "NULL") {
    startAP();
  } else {
    startSTA();
  }
}

void loop() {
  server.handleClient();
  delay(2); // Pozwala systemowi operacyjnemu ESP na obsługę procesów w tle
}

void startAP() {
  Serial.println("\nUruchamiam AP: ESP32-Manager");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Manager");

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlAP);
  });

  server.on("/save", HTTP_POST, []() {
    // Odczyt danych z formularza
    String s = server.arg("ssid");
    String p = server.arg("pass");
    
    Serial.print("Odebrano SSID: "); Serial.println(s);
    
    if (s.length() > 0) {
      preferences.putString("ssid", s);
      preferences.putString("pass", p);
      server.send(200, "text/html", "Zapisano. Restartuje...");
      delay(2000);
      ESP.restart();
    } else {
      server.send(200, "text/plain", "Blad: SSID nie moze byc puste!");
    }
  });
  
  server.begin();
}

void startSTA() {
  Serial.printf("\nLaczenie z: %s\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(1000);
    Serial.print(".");
    counter++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nPolaczono!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, []() {
      server.send(200, "text/html", "<h1>Polaczono!</h1><a href='/reset'><button>Resetuj Wi-Fi</button></a>");
    });

    server.on("/reset", HTTP_GET, []() {
      preferences.clear();
      server.send(200, "text/plain", "Skasowano dane. Restart...");
      delay(1000);
      ESP.restart();
    });
    
    server.begin();
  } else {
    Serial.println("\nBlad polaczenia. Wraca do AP.");
    preferences.clear(); // Czyścimy błędne dane
    startAP();
  }
}
