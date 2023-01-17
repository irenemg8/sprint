// ---------------------------------------------------
// Sensores.h
// ---------------------------------------------------
#ifndef SENSORES_YA_INCLUIDO
#define SENSORES_YA_INCLUIDO
// ---------------------------------------------------
#include <Adafruit_ADS1X15.h>                   //Incluimos la librería
Adafruit_ADS1X15 ads1115; // declaramos el ADS1115

//------ Declaramos las variables y las conexiones ------------------------------------
double adc0;
double adc1;
double adc2;
double adc3;
double vo;          // Valor de la tensión*/

double Ph; // Cables azules
double Humedad; // Cables marrones
double Salinidad; // Cables verdes
double Temperatura; // Cables morado
double Luminosidad;

// Cables a tierra -> naranja
// Cables a voltaje -> amarillo

#define power_pin 
int channelValue = 0;
int sensorValue = 0;
int Sal = 0;
//pinMode(5, OUTPUT);


class Sensores{
  private:
    //--- variables Humedad ---
    const int AirValue = 30130;   // Medimos valor en seco
    const int WaterValue = 24575; // Medimos valor en agua

    //--- variables Salinidad ---
    const int SaltValue = 2600;        // Valor con sal
    const int WithoutSaltValue = 1800; // Valor sin sal


    //--- variables pH ---
    int channelValue = 0;
    int Offset = 0;
    int samplingInterval = 20;
    int printInterval = 800;
    int ArrayLength = 40;     // numero de muestras
    int pHArray[40]; // almacena las muestras
    int pHArrayIndex = 0;


  public:
    Sensores();                  // declaro la clase
    double funcionHumedad();     // Función humedad
    double funcionph();          // Función pH
    double funcionSalinidad();   // Función salinidad
    double funcionTemperatura(); // Función temperatura
    double funcionLuminosidad(); // Función luminosidad

}; // class
// ---------------------------------------------------
#endif



Sensores::Sensores(){
}


//--------------------------------------------  
// HUMEDAD      - adc 0 -
//---------------------------------------------
double Sensores::funcionHumedad(){
 int adc0 = ads1115.readADC_SingleEnded(0);
  double Humedad = 100*AirValue/(AirValue-WaterValue)-adc0*100/(AirValue-WaterValue);
  //  Humedad=map(adc0,529,540,0,100);
  if (Humedad > 100){
    Humedad = 100;
  }
  if (Humedad < 5){
    Humedad = 0;
  }
return Humedad;
}



//--------------------------------------------  
// PH          - adc1 -
//---------------------------------------------
double Sensores::funcionph(){
  static unsigned long samplingTime = millis();
  //static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval) {
    pHArray[pHArrayIndex++] = ads1115.readADC_SingleEnded(1);
    if (pHArrayIndex == ArrayLength){
      pHArrayIndex = 0;
      double counter = 0.0;
      for (int i = 0; i < ArrayLength; i++){
        counter = pHArray[i] + counter; 
      } 
    }
    
    double valueIpH = ads1115.readADC_SingleEnded(1);
    //Convertir la lectura en tension


    voltage = (4.096 / 32767.0) * valueIpH;
    Ph = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  return Ph;
}




//--------------------------------------------  
// SALINIDAD
//---------------------------------------------
double Sensores::funcionSalinidad(){                             //Muestrear la tensión del sensor de salinidad      
  int16_t adc0;
  digitalWrite(5, HIGH);
  delay(1000);

  adc0 = analogRead(A0);
  digitalWrite(5, LOW);
  delay(1000);
   //mapeamos el valor leido para un porcentaje entre 0 a 100
  Salinidad=map(adc0,540,910,0,100);
  //limitamos los valores entre 0 y 100 para minimizar errores
  if(Salinidad>=100)
  {
    Salinidad=100;
  }
  if(Salinidad<=0)
  {
    Salinidad=0;
  }
  return (Salinidad);
}


//--------------------------------------------  
// TEMPERATURA     - adc2 -
//---------------------------------------------
double Sensores::funcionTemperatura(){
int16_t adc2 = ads1115.readADC_SingleEnded(2);  
  double m = 37*pow(10, -3);
  double b = 0.79;
  double vo = (adc2 * 4.096) / 32767;             
  double Temperatura = ((vo-b)/m);
return Temperatura;
}

//--------------------------------------------  
// LUMINOSIDAD      - adc 3 -
//---------------------------------------------
double Sensores::funcionLuminosidad(){
  int adc3=ads1115.readADC_SingleEnded(3);
  double Luminosidad=(adc3*4.096)/32767;
return Luminosidad;
}
