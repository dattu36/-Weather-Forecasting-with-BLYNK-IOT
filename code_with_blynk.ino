#define BLYNK_TEMPLATE_ID "TMPLW80uXNJH"
#define BLYNK_DEVICE_NAME "Weather Monitoring System"
#define BLYNK_AUTH_TOKEN "Wz0Y-KSuicuQfVbt0Sn9y_xDsyqMzCGz"
 
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
  
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "hannah";
char pass[] = "hannah30";

//char ssid[] = "iot";
//char pass[] = "1234567800";

int buzzer=2;

#define DHT11_PIN 5
//DHT11 data pin to D4
DHT dht(DHT11_PIN,DHT11);
#define rain 2 
 
BlynkTimer timer; 
#include <LiquidCrystal_I2C.h> 

#define mq135 32

//String long_lat;
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27, 16, 2);

 
// You will need to create an SFE_BMP180 object, here called "pressure":
 
SFE_BMP180 pressure;
//Hardware connections:
//NodeMCU     BMP180
//3.3V         VIN
//GND          GND
//D1           SCL          
//D2           SDA            
//(WARNING: do not connect + to 5V or the sensor will be damaged!)
 
 
#define ALTITUDE 1655.0 // Altitude in meters 
 
void setup()
{
  //Init serial
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  
  //Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
    lcd.begin(16, 2);
    pinMode(buzzer,OUTPUT);
    pinMode(rain,INPUT);
    pinMode(mq135,INPUT); 
    dht.begin(); 

      if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}


void loop()
{
 
Blynk.run();
timer.run();
 
   
    float h = dht.readHumidity();
    float t = dht.readTemperature();         
    Serial.print("Current humidity = ");
    Serial.print(h);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(t); 
    Serial.println("C  ");
 //lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("T:");
   lcd.print(t);

   lcd.setCursor(8,0);
   lcd.print("H:");
   lcd.print(h);

    Blynk.virtualWrite(V2,t);
    Blynk.virtualWrite(V3,h);

   int rainval=digitalRead(rain);
   if(rainval==0)
   {   
    lcd.setCursor(0,1);
   lcd.print("R:Raining");
   Blynk.virtualWrite(V0,"Raining");
   }
   else
   {
   lcd.setCursor(0,1);
   lcd.print("R:NoRain ");
   Blynk.virtualWrite(V0,"NoRain ");
    }


     
  char status;
  double T,P,p0,a;
 
  // Loop here getting pressure readings every 10 seconds.
 
  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
   
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
   lcd.setCursor(10,1);
   lcd.print("P:");
   lcd.print(ALTITUDE*3.28084,0);
   
   Blynk.virtualWrite(V1,ALTITUDE*3.28084);
  

  
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.
 
  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
 
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
 
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.
 
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
 
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
 
        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.
 
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
     
          Serial.print(" mb, ");

          
          Serial.print(P*0.0295333727,2);
//          Blynk.virtualWrite(V6,P*0.0295333727,2);
          Serial.println(" inHg");
 
          // The pressure sensor returns absolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sea level function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb
 
          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
    
          
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2); 
          
          Serial.println(" inHg");
 
          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.
 
          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0); 
           
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0); 
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
 
  delay(5000);  // Pause for 5 seconds.
    
 
 }
  
