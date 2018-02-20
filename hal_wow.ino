#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#define  MAX_ATTEMPT  5

const char* ssid = "YourWifiSSID";  // Change to your own wifi
const char* password = "YourWifiPassword"; // Change to your own password

static int attempted;

ESP8266WebServer server(80);  // Change to your own http port

const char* auth_user = "Kevin";  // Change to your own login ID
const char* auth_password = "password123";  // Change to your own password

int power_control_pin = D7;   //GPIO 13
int power_status_pin = D1;    //GPIO 5

const static char html_str[] =   "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  <title>Control</title><style>body{background-color:lightblue;font-size:24px;}</style></head><body><h1>Control</h1>\
  <ul><li><a href=\"on\">TURN ON</a></li><br><li><a href=\"off\">TURN OFF</a></li><br><li><a href=\"force\">FORCE SHUTDOWN\
  </a></li><br><li><a href=\"check\">CHECK POWER STATUS</a></li><br></ul><i>%s</i></body></html>";

void setup() {

  attempted = 0;

  // Set static ip
  WiFi.persistent( false );
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  IPAddress ip(192, 168, 1, 253);     // Your static IP
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8); 

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  // For console debug
  Serial.begin(115200);


  // Wifi connection start
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);


  // GPIO Initialize
  pinMode(power_control_pin, OUTPUT);
  pinMode(power_status_pin, INPUT);

  // For security reason, only respond to "/wow" page
  server.on("/wow", []() {
    if (!server.authenticate(auth_user, auth_password)) {
      if (attempted >= MAX_ATTEMPT) {
        Serial.println("blocked");
        delay(30000); // blocking login for 30 seconds
        attempted = 0;
      } else {
        attempted++;
        Serial.println(String(attempted) + " attempted.");
      }
      return server.requestAuthentication();
    }
    //server.send(200, "text/html", "Login OK, IP: " + server.client().remoteIP().toString()  );
    attempted = 0;
    char temp[1024];
    sprintf(temp, html_str, "> Choose an operation");
    server.send(200, "text/html", temp);
  });

  server.on("/force", []() {
    if (!server.authenticate(auth_user, auth_password)) {
      Serial.println("Not authurized.");
      return;
    } else {
      char temp[1024];
      sprintf(temp, html_str, "> Force shutdown processed!");
      server.send(200, "text/html", temp);

      digitalWrite(power_control_pin, HIGH);
      delay(15000); // hold for 15 seconds
      digitalWrite(power_control_pin, LOW);
      
      Serial.println("Force shutdown processed!");
    }
  });

  server.on("/off", []() {
    if (!server.authenticate(auth_user, auth_password)) {
      Serial.println("Not authurized.");
      return;
    } else {
      char temp[1024];
      sprintf(temp, html_str, "> Power off processed!");
      server.send(200, "text/html", temp);

      if(digitalRead(power_status_pin) == HIGH) {
        digitalWrite(power_control_pin, HIGH);
        delay(500); // hold for 500ms
        digitalWrite(power_control_pin, LOW);
      }
      Serial.println("Power off processed!");
    }
  });

  server.on("/on", []() {
    if (!server.authenticate(auth_user, auth_password)) {
      Serial.println("Not authurized.");
      return;
    } else {
      char temp[1024];
      sprintf(temp, html_str, "> Power on processed!");
      server.send(200, "text/html", temp);

      if(digitalRead(power_status_pin) == LOW) {
        digitalWrite(power_control_pin, HIGH);
        delay(500); // hold for 500ms
        digitalWrite(power_control_pin, LOW);
      }
      
      Serial.println("Power on processed!");
    }
  });

  server.on("/check", []() {
    if (!server.authenticate(auth_user, auth_password)) {
      Serial.println("Not authurized.");
      return;
    } else {
      char temp[1024];
      if(digitalRead(power_status_pin) == HIGH){
        sprintf(temp, html_str, "> Power is ON!");      
      }else {
        sprintf(temp, html_str, "> Power is OFF!");
      }
          
      server.send(200, "text/html", temp);
      Serial.println("Checking power status!");
    }
  });
  
  // Override NotFound handler and ignore other requests
  server.onNotFound(handleNotFound);

  // Run http server
  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void loop() {
  server.handleClient();
}

void handleNotFound() {
  // Ignore other requests
}


