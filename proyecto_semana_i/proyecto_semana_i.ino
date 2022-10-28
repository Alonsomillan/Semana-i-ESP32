/***FIREBASE DATABASE***/
#include <esp_wpa2.h>
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
const char* ssid = "Tec"; 
#define EAP_IDENTITY "a01328817@tec.mx"
#define EAP_PASSWORD "...................."

// Insert Firebase project API Key
#define API_KEY "AIzaSyBTPd0GnHX4n4pI-S50kxixFoMl8ILaVy4"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://anthony-pruebas-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

/***DHT11***/
#include "DHT.h"
#define DHTPIN 33     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

float h = 0;
float t = 0;
float f = 0;

void dht11_sensor()
{
    /****DTH11**/
    // Wait a few seconds between measurements.
    delay(2000);
  
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    f = dht.readTemperature(true);
  
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) 
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  
    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);
  
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));
}

/***SENSOR ULTRASÓNICO***/
const int trigPin = 26;
const int echoPin = 27;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void sensor_ultrasonico()
{
    /***SENSOR ULTRASÓNICO***/
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
  
    // Calculate the distance
    distanceCm = duration * SOUND_SPEED/2;
  
    // Convert to inches
    distanceInch = distanceCm * CM_TO_INCH;
  
    // Prints the distance in the Serial Monitor
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
    Serial.print("Distance (inch): ");
    Serial.println(distanceInch);
  
    delay(1000);
}

/***PIR***/
const int PIN_TO_SENSOR = 32; // GIOP19 pin connected to OUTPUT pin of sensor
int pinStateCurrent   = LOW;  // current state of pin
int pinStatePrevious  = LOW;  // previous state of pin
int alarma;

void sensor_pir()
{
    /***PIR**/
    pinStatePrevious = pinStateCurrent; // store old state
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state
  
    if (pinStatePrevious == LOW && pinStateCurrent == HIGH) 
    {   // pin state change: LOW -> HIGH
      Serial.println("Motion detected!");
      // TODO: turn on alarm, light or activate a device ... here
      alarma = 1;
      Serial.println(alarma);
      delay(2000);
    }
    else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) 
    {   // pin state change: HIGH -> LOW
      Serial.println("Motion stopped!");
      // TODO: turn off alarm, light or deactivate a device ... here
      alarma= 0;
      Serial.println(alarma);
    }
}

/***PWM LED**/
// the number of the LED pin
const int ledPin = 25; // 16 corresponds to GPIO16 se cambió a GPIO15

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
String pwmValue;

void pwm_led()
{
    /***PWM LED***/
    // increase the LED brightness
    for(int dutyCycle = 0; dutyCycle <= pwmValue.toInt(); dutyCycle++)
    {
      // changing the LED brightness with PWM
      ledcWrite(ledChannel, dutyCycle);
      delay(15);
    }
    
    // decrease the LED brightness
    for(int dutyCycle = pwmValue.toInt(); dutyCycle >= 0; dutyCycle--)
    {
      // changing the LED brightness with PWM
      ledcWrite(ledChannel, dutyCycle);
      delay(15);
    }
}

/***SEVEN SEGMENT DISPLAY CÁTODO COMÚN***/
//se demuestra el manejo del display de siete segmentos

//se declaran los pines a usar
//int LEDs[] = {8,9,7,6,4,3,2};        // for Arduino microcontroller
//int LEDs[] = {D2,D1,D3,D4,D6,D7,D8}; // for ESP8266 microcontroller
int LEDs[] = {22,23,4,3,19,18,5};    // for ESP32 microcontroller

//se declaran los arreglos que forman los dígitos
int zero[] = {0, 1, 1, 1, 1, 1, 1};   // cero
int one[] = {0, 0, 0, 0, 1, 1, 0};   // uno
int two[] = {1, 0, 1, 1, 0, 1, 1};   // dos
int three[] = {1, 0, 0, 1, 1, 1, 1};   // tres
int four[] = {1, 1, 0, 0, 1, 1, 0};   // cuatro 
int five[] = {1, 1, 0, 1, 1, 0, 1};   // cinco
int six[] = {1, 1, 1, 1, 1, 0, 1};   // seis
int seven[] = {1, 0, 0, 0, 1, 1, 1};   // siete
int eight[] = {1, 1, 1, 1, 1, 1, 1}; // ocho
int nine[] = {1, 1, 0, 1, 1, 1, 1};   // nueve
int ten[] = {1, 1, 1, 0, 1, 1, 1};   // diez, A
int eleven[] = {1, 1, 1, 1, 1, 0, 0};   // once, b
int twelve[] = {0, 1, 1, 1, 0, 0, 1};   // doce, C
int thirteen[] = {1, 0, 1, 1, 1, 1, 0};   // trece, d
int fourteen[] = {1, 1, 1, 1, 0, 0, 1};   // catorce, E
int fifteen[] = {1, 1, 1, 0, 0, 0, 1};   // quince, F

//se declara contador
unsigned char contador = 0;

//función que despliega del 0 al F
void segment_display(unsigned char valor)
{
    switch(valor)
    {
        case 0:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], zero[i]);
                    break;
        case 1:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], one[i]);
                    break;
        case 2:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], two[i]);
                    break;
        case 3:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], three[i]);
                    break;
        case 4:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], four[i]);
                    break;
        case 5:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], five[i]);
                    break;
        case 6:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], six[i]);
                    break;
        case 7:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], seven[i]);
                    break;
        case 8:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], eight[i]);
                    break;
        case 9:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], nine[i]);
                    break;
        case 10:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], ten[i]);
                    break;
        case 11:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], eleven[i]);
                    break;
        case 12:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], twelve[i]);
                    break;
        case 13:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], thirteen[i]);
                    break;
        case 14:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], fourteen[i]);
                    break;
        case 15:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], fifteen[i]);
                    break; 
        default:
                    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], zero[i]);
                    break;          
    }
}

String intValue;

void seven_segment_display_catodo_comun()
{
     /***SEVEN SEGMENT DISPLAY CÁTODO COMÚN***/
    //se muestra contador, retardo y se incrementa contador
    segment_display(contador);
    if(contador < 15) contador = intValue.toInt();
    else contador = 0;
}

/***TOUCH SENSOR***/
// set pin numbers
const int touchPin = 14; 
const int ledPinTouch = 2;

// change with your threshold value
const int threshold = 20;
// variable for storing the touch pin value 
int touchValue;
bool tocar = false;

void touch_sensor()
{
  // read the state of the pushbutton value:
  touchValue = touchRead(touchPin);
  Serial.print(touchValue);
  // check if the touchValue is below the threshold
  // if it is, set ledPinTouch to HIGH
  if(touchValue < threshold){
    // turn LED on
    tocar = true;
    Serial.println(tocar);
    digitalWrite(ledPinTouch, HIGH);
    Serial.println(" - LED on");
  }
  else{
    // turn LED off
    tocar = false;
    Serial.println(tocar);
    digitalWrite(ledPinTouch, LOW);
    Serial.println(" - LED off");
  }
  delay(15);
}

/***VARIABLES**/
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
//bool boolValue;

void setup() 
{
  //Serial begin
  Serial.begin(115200);

  /***CONECTAR WIFI Y FIREBASE DATABASE***/
  // WPA2 enterprise magic starts here
    WiFi.disconnect(true);      
    WiFi.mode(WIFI_STA);   //init wifi mode
    Serial.printf("Connecting to WiFi: %s ", ssid);
    //esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)incommon_ca, strlen(incommon_ca) + 1);
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    //esp_wifi_sta_wpa2_ent_enable();
    esp_wpa2_config_t configW = WPA2_CONFIG_INIT_DEFAULT();
    esp_wifi_sta_wpa2_ent_enable(&configW);
    // WPA2 enterprise magic ends here
    WiFi.begin(ssid);
  
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  
  /***SEVEN SEGMENT DISPLAY CÁTODO COMÚN***/
  //se inicializan los pines como salida
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT);
  
  /****DTH11**/
  Serial.println(F("DHTxx test!"));
  dht.begin();

  /***SENSOR ULTRASÓNICO***/
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  /***PIR***/
  pinMode(PIN_TO_SENSOR, INPUT); // set ESP32 pin to input mode to read value from OUTPUT pin of sensor

  /***PWM LED***/
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);

  /***TOUCH SENSOR***/
  pinMode (ledPinTouch, OUTPUT);

}

void loop() 
{  
    sensor_pir();
    touch_sensor();

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

//        /***CONTADOR SEVEN SEGMENT DISPLAY CÁTODO COMÚN***/
//        // Write an Int number on the database path test/contador
//        if (Firebase.RTDB.setInt(&fbdo, "esp32/contador", contador))
//        {
//            Serial.println("PASSED");
//            Serial.println("PATH: " + fbdo.dataPath());
//            Serial.println("TYPE: " + fbdo.dataType());
//        }
//        else 
//        {
//            Serial.println("FAILED");
//            Serial.println("REASON: " + fbdo.errorReason());
//        }
//        seven_segment_display_catodo_comun();

        /****DTH11**/
        dht11_sensor();
        // Write an Float number on the database path test/temperaturaC
        if (Firebase.RTDB.setFloat(&fbdo, "esp32/temperaturaC", t))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        // Write an Float number on the database path test/temperaturaF
        if (Firebase.RTDB.setFloat(&fbdo, "esp32/temperaturaF", f))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        // Write an Float number on the database path test/humedad
        if (Firebase.RTDB.setFloat(&fbdo, "esp32/humedad", h))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        /***SENSOR ULTRASÓNICO***/
        sensor_ultrasonico();
        // Write an Float number on the database path test/distanciaCm
        if (Firebase.RTDB.setFloat(&fbdo, "esp32/distanciaCm", distanceCm))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        // Write an Float number on the database path test/distanciaInch
        if (Firebase.RTDB.setFloat(&fbdo, "esp32/distanciaInch", distanceInch))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        /***PIR***/
        // Write an Float number on the database path test/alarma
        if (Firebase.RTDB.setInt(&fbdo, "esp32/presencia", alarma))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        /***TOUCH SENSOR***/
        // Write an Int number on the database path test/touch
        if (Firebase.RTDB.setBool(&fbdo, "esp32/touch", tocar))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
        }
        else 
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }


        /***SEVEN SEGMENT DISPLAY CÁTODO COMÚN***/
        //Lee el dato digito path test/digito
        if (Firebase.RTDB.getString(&fbdo, "/esp32/digito")) 
        {
            if (fbdo.dataType() == "string") 
            {
                intValue = fbdo.stringData();
                Serial.println(intValue);
            }
        }
        else 
        {
            Serial.println(fbdo.errorReason());
        }
        seven_segment_display_catodo_comun();

        /***PWM LED***/
        //lee segundo dato
        if (Firebase.RTDB.getString(&fbdo, "/esp32/pwm")) 
        {
            if (fbdo.dataType() == "string") 
            {
                pwmValue = fbdo.stringData();
                Serial.println(pwmValue);
            }
        }
        else 
        {
            Serial.println(fbdo.errorReason());
        }
        pwm_led();
    }
}
