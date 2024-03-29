#include <Wire.h> 
#include <LiquidCrystal_I2C.h>                  //Includimos la librería lcd
#include <ESP8266WiFi.h>                        //Incluimos la librería de la WIFI
#include "Sensores.h"                           //Nuestra librería de sensores
#include <TinyGPS++.h>                          //Librerías gps
#include <SoftwareSerial.h>

//----------------LCD-------------------------------------------------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // La LCD tiene 16 columnas y 2 filas
//----------------------------------------------------------------------------------------

//El circuito de humedad se conecta al puerto 0 (adc0)
//El circuito de pH se conecta al puerto 1 (adc1)
//El circuito de temperatura se conecta al puerto 2 (adc2)
//El circuito de luminosidad se conecta al puerto 3 (adc3)

/* Programa prueba GPS UBLOX-6M
 *  Conexiones GPIO 12--> TX
 *             GPIO 13--> RX*/
SoftwareSerial gps(12,13);
char dato;



//---------------- DECLARACIÓN SENSORES ----------------------------------------------
Sensores s;      //Sensores

//------------ WIFI ------------------------------------------------------------------
// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
//#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/1935509)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (http://dweet.io/follow/PruebaGTI)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "Irene";              //iPhone de Elena
  const char WiFiPSK[] = "0208KEY1402TE";        //1234567890
#endif



///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="98S01TPNU1JL3IM7"; // Escribe la clave de tu canal ThingSpeak   98S01TPNU1JL3IM7    Q4KUZ1HQEV21LEIG
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="PruebaGTI"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 4 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}




//-------------------------------------------------------------------------------------
//---- Void setup ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando...");
  ads1115.begin();                                    //Inicializa ads1115
  Serial.println("Ajustando la ganancia...");         //Ajusta ganancia a GAIN ONE
  ads1115.setGain(GAIN_ONE);
  Serial.println("Tomando medidas..."); 

//-------------- LCD --------------------------------------------------------------------
   lcd.init();                                      
   lcd.setCursor(0, 0);
   lcd.backlight();
   lcd.print("Inicializando...");
   lcd.setCursor(0, 1);
   lcd.print("Buenos Dias :)");
   delay(4000);
   lcd.clear();

//-------------- GPS --------------------------------------------------------------------           
 gps.begin(9600); 
 Serial.println("Inicializando el GPS...");
 delay(500);
 Serial.println("Esperando datos");


// ----------- WIFI ---------------------------------------------------------------------
 #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(115200);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif

  
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------



//---------------------------------------------------  
//------- Void loop ---------------------------------

void loop(){
Humedad = s.funcionHumedad();
Ph = s.funcionph();
Salinidad = s.funcionSalinidad();
Temperatura = s.funcionTemperatura();
Luminosidad = s.funcionLuminosidad();


if(gps.available()){
   Serial.print(gps.read());
   delay(100);
}


//-------------------- WIFI --------------------------
String data[ 5 + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String(Ph); //Escribimos el dato de pH
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "PH = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = String(Humedad); //Escribimos el dato de humedad
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.println( data[ 2 ] );
    #endif

    data[ 3 ] = String(Salinidad); //Escribimos el dato de salinidad
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.println( data[ 3 ] );
    #endif

    data[ 4 ] = String(Temperatura); //Escribimos el dato de temperatura
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Temperatura = " );
        Serial.println( data[ 4 ] );
    #endif

     data[ 5 ] = String(Temperatura); //Escribimos el dato de temperatura
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Luminosidad = " );
        Serial.println( data[ 5 ] );
    #endif
    
    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
//   delay( 15000 );   
//   Serial.print( "Goodnight" );
//   ESP.deepSleep( sleepTimeSeconds * 1000000 );




//-----------------------------------------------------------------------
//---------------LCD-----------------------------------------------------

 // Limpiamos la pantalla
  lcd.clear();

  // Situamos el cursor en la columna 0 fila 0
  lcd.setCursor(0,0);
  lcd.print("Humedad:");                          // Escribimos lo que va a mostrar en pantalla
 
  // Situamos el cursor en la columna 0 fila 1
  lcd.setCursor(0,1);
  lcd.print(Humedad);                              // Escribimos lo que va a mostrar en pantalla

  delay(2000);
  
//------------- Dejamos un tiempo para leerlo ----------------------------
  // Limpiamos la pantalla
  lcd.clear();

  // Situamos el cursor en la columna 0 fila 0
  lcd.setCursor(0,0);
  lcd.print("Salinidad:");                          // Escribimos lo que va a mostrar en pantalla
 
  // Situamos el cursor en la columna 0 fila 1
  lcd.setCursor(0,1);
  lcd.print(Salinidad);                              // Escribimos lo que va a mostrar en pantalla

  delay(2000);
  
//------------- Dejamos un tiempo para leerlo ----------------------------
  // Limpiamos la pantalla
  lcd.clear();

  // Situamos el cursor en la columna 0 fila 0
  lcd.setCursor(0,0);
  lcd.print("Ph:");                          // Escribimos lo que va a mostrar en pantalla
 
  // Situamos el cursor en la columna 0 fila 1
  lcd.setCursor(0,1);
  lcd.print(Ph);                              // Escribimos lo que va a mostrar en pantalla

  delay(2000);
  
//------------- Dejamos un tiempo para leerlo ----------------------------
  // Limpiamos la pantalla
  lcd.clear();

  // Situamos el cursor en la columna 0 fila 0
  lcd.setCursor(0,0);
  lcd.print("Temperatura:");                          // Escribimos lo que va a mostrar en pantalla
 
  // Situamos el cursor en la columna 0 fila 1
  lcd.setCursor(0,1);
  lcd.print(Temperatura);                              // Escribimos lo que va a mostrar en pantalla

  delay(2000);
  
//------------- Dejamos un tiempo para leerlo ----------------------------

// Limpiamos la pantalla
  lcd.clear();

  // Situamos el cursor en la columna 0 fila 0
  lcd.setCursor(0,0);
  lcd.print("Luminosidad:");                          // Escribimos lo que va a mostrar en pantalla
 
  // Situamos el cursor en la columna 0 fila 1
  lcd.setCursor(0,1);
  lcd.print(Luminosidad);                              // Escribimos lo que va a mostrar en pantalla

  delay(2000);
  
//------------- Dejamos un tiempo para leerlo ----------------------------




  // ---------------------------------------------------------------------
  // ------ Imprimir en pantalla con el monitor serie --------------------
  // ---------------------------------------------------------------------
  // Escribimos el pH
  // ---------------------------------------------------------------------
 // Serial.println(adc0);
  Serial.print("pH: ");
  Serial.println(Ph);
  // ---------------------------------------------------------------------
  // Escribimos la humedad
  // ---------------------------------------------------------------------
 // Serial.println(adc1);
  Serial.print("Humedad (%): ");
  Serial.print(Humedad);
  Serial.println("%");
  // ---------------------------------------------------------------------
  // Escribimos la salinidad
  // ---------------------------------------------------------------------
 // Serial.println(adc2);
  Serial.print("Salinidad (%): ");
  Serial.print(Salinidad);
  Serial.println("%");
  // ---------------------------------------------------------------------
  // Escribimos la temperatura
  // ---------------------------------------------------------------------
  Serial.print("Temperatura(º): ");
  Serial.print(Temperatura);
  Serial.println("º");
   // ---------------------------------------------------------------------
  // Escribimos la luminosidad
  // ---------------------------------------------------------------------
  Serial.print("Intensidad lumínica: ");
  Serial.println(Luminosidad);


//deepSleep(3000);            //DeepSleep cada 3 segundos
                            // unir los pines 16 y reset después de haber subido el código a la placa

}
