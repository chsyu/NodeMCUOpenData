#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char* ssid     = "";           // insert your SSID
const char* password = "";   // insert your password

WiFiClient client;

const char* server = "opendata2.epa.gov.tw";  // server's address
const char* resource = "/AQX.json"; 

char response[22000], parse_res[500]; // this fixed sized buffers works well for this project using the NodeMCU.

void setup(){
    delay(3000);
    // initialize serial
    Serial.begin(250000);
      
    // initialize WiFi
    WiFi.begin(ssid, password);

    //Connection WiFi    
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("Wi-Fi is Connected!!!");
}

void loop(){
    
        // connect to server  
        bool ok = client.connect(server, 80);
        while(!ok){
            Serial.print(".");
            delay(500);        
        }
        Serial.println("");
        Serial.println("Client is Connected!!!");
    
        //Send request to resource
        client.print("GET ");
        client.print(resource);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("Connection: close");
        client.println();
    
        delay(100);
    
        //Reading stream and remove headers
        client.setTimeout(5000);
        client.readBytes(response, 5000);
        
        // process JSON
        DynamicJsonBuffer jsonBuffer;
        int bgni = 0;
        int endi = 0;
        int cnti = 0;
        bool siteFound = false;
        char* location, site, pm25, psi;
        char* siteName = "士林";

        while(!siteFound) {
          bool beginFound = false;
          bool endFound = false;
          while(!beginFound || !endFound){
              cnti++;
              if(int(response[cnti]) == 123) { // check for the "{" 
                beginFound = true;
                bgni = cnti;
              } else if(int(response[cnti]) == 125) { // check for the "}" 
                endFound = true;
                endi = cnti;
              }
          }
          
          int eol = sizeof(response);
  
          //restructure by shifting the correct data
//          Serial.println("restructure");
          for(int c=0; c<(endi-bgni+1); c++){
              parse_res[c] = response[((c+bgni))];
//            Serial.print(parse_res[c]);
          }
          
          JsonObject& root = jsonBuffer.parseObject(parse_res);
          
//          if (!root.success()) {
//            Serial.println("JSON parsing failed!");
//          } 
//          else {
//            Serial.println("JSON parsing worked!");
//          }
          
          const char* location = root["County"];
          const char* site = root["SiteName"];
          const char* pm25 = root["PM2.5"]; 
          const char* psi = root["PSI"];
          if(strcmp(site, siteName) == 0) {
            siteFound = true;
            // Print data to Serial
            Serial.print(location);
            Serial.print(" ");
            Serial.print(site);
            Serial.println("區");
            Serial.print("PM2.5: ");
            Serial.println(pm25);
            Serial.print("PSI: ");
            Serial.println(psi);
            Serial.println("----------"); 
          }
        }
     
        client.stop(); // disconnect from server
    
        for(int x=0; x<10; x++){ // wait for new connection with progress indicator
            Serial.print(".");
            delay(1000); // the OWM free plan API does NOT allow more then 60 calls per minute
        }
        
        Serial.println("");
     
    }
