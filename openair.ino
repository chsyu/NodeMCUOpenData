#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char* ssid     = "";           // insert your SSID
const char* password = "";   // insert your password

WiFiClient client;

const char* server = "opendata2.epa.gov.tw";  // server's address
const char* resource = "/AQX.json"; 

char response[30000], parse_res[1500]; // this fixed sized buffers works well for this project using the NodeMCU.

void setup(){
    delay(3000);
    // initialize serial
    Serial.begin(115200);
    while(!Serial){
        delay(100);
        }
      
    // initialize WiFi
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
    }
   
}



void loop(){
    
        // connect to server  
        bool ok = client.connect(server, 80);
        bool beginFound = false;
        bool endFound = false;
        
        while(!ok){
            //Serial.print(".");
            delay(500);        
        }
    
        delay(500);
    
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
        client.readBytes(response, 25000);
        
        // process JSON
        DynamicJsonBuffer jsonBuffer;
        
        int bgni = 0;
        int endi = 0;
        int cnti = 0;
        while(!beginFound || !endFound){
            cnti++;
            if(int(response[bgni]) == 123){ // check for the "{" 
              beginFound = true;
//              bgni = cnti;
            }
            if(int(response[endi]) == 125){ // check for the "}" 
              endFound = true;
//              endi = cnti;
            }
           if(!beginFound) {
            bgni++;
           }
           if(!endFound) {
            endi++;
           }
        }
        
        int eol = sizeof(response);
        Serial.print("Length = ");
        Serial.println(eol);
        Serial.print("bgni = ");
        Serial.println(bgni);
        Serial.print("endi = ");
        Serial.println(endi);

        //restructure by shifting the correct data
        Serial.println("restructure");
        for(int c=0; c<(endi-bgni+1); c++){
            parse_res[c] = response[((c+bgni))];
            Serial.print(parse_res[c]);
        }
 
        Serial.println("Done...");


        
        JsonObject& root = jsonBuffer.parseObject(parse_res);
        
        if (!root.success()) {
          Serial.println("JSON parsing failed!");
        } 
        else {
          Serial.println("JSON parsing worked!");
        }

//        JsonObject& rootAir = root[0];
        
        const char* location = root["County"];
        const char* site = root["SiteName"];
        const char* pm25 = root["PM2.5"]; 
        const char* psi = root["PSI"];
        // Print data to Serial
        Serial.print("*** ");
        Serial.print(location);
        Serial.print("   ");
        Serial.println(site);
        Serial.print("PM2.5: ");
        Serial.println(pm25);
        Serial.print("PSI: ");
        Serial.println(psi);
        Serial.println("----------"); 
     
        client.stop(); // disconnect from server
    
        for(int x=0; x<10; x++){ // wait for new connection with progress indicator
            Serial.print(".");
            delay(100); // the OWM free plan API does NOT allow more then 60 calls per minute
        }
        
        Serial.println("");
     
    }
