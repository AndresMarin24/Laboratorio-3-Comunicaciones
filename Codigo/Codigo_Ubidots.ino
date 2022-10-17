
//Se incluye la libreria de Ubidots
#include "UbidotsEsp32Mqtt.h"

//Se define el Token de Ubidots a usar
const char *UBIDOTS_TOKEN = "BBFF-7V906eOlXXwdQoEJzP9H9QarMuPtio";
Ubidots ubidots(UBIDOTS_TOKEN);

//Se define la Red wifi que se usara, tanto el SSID o nombre de la red como la contraseña
const char *WIFI_SSID = "Sara";     
const char *WIFI_PASS = "hellomoto"; 

//Se pone el tablero de UBidots que se va a trabajar y las variables que se manipularan para recepcion y envio de datos
const char *DEVICE_LABEL = "Lab4Com";  
const char *VARIABLE_LABEL = "LedRojo";
const char *VARIABLE_LABEL_1 = "LedAmarillo";
const char *VARIABLE_LABEL_2 = "LedVerde";
const char *VARIABLE_LABEL_3 = "HALL"; 
const char *VARIABLE_LABEL_4 = "NTC";
const char *VARIABLE_LABEL_5 = "TIEMPO"; 

//Se define la frecuencia de envio
const int PUBLISH_FREQUENCY = 1000; 

//Se ponen los pines de los leds del semaforo
const uint8_t Pin_Led_Rojo = 5; 
const uint8_t Pin_Led_Amarillo = 19;
const uint8_t Pin_Led_Verde = 18;

//Se ponen el pin del rele
int Pin_rele = 13; 

//Variables para el sensor de efecto Hall
int Hall;
float voltA=0;
double gauss=0;
double teslas=0;

//Variables para el sensor de temperatura LM35
float Lm35;
float voltB=0;

//Variables auxiliares
uint32_t long timer;
uint32_t long Temp_Aux;
unsigned long tiempo_actual;
unsigned long tiempo_anterior=0;
unsigned long delta_tiempo;
int contador=0;
double Aux=0;
const int Text_Telegram = 33;

//Funcion para recepcion de datos desde Ubidots
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    if ((char)payload[0] == '1'){
      digitalWrite(Pin_Led_Rojo, LOW);
    }else if((char)payload[0] == '2'){
      digitalWrite(Pin_Led_Rojo, HIGH);
    }else if((char)payload[0] == '3'){
      digitalWrite(Pin_Led_Amarillo, LOW);
    }else if((char)payload[0] == '4'){
      digitalWrite(Pin_Led_Amarillo, HIGH);
    }else if((char)payload[0] == '5'){
      digitalWrite(Pin_Led_Verde, LOW);
    }else if((char)payload[0] == '6'){
      digitalWrite(Pin_Led_Verde, HIGH);
    }
  }
  Serial.println();
}

//Seteo de configuracion y condiciones iniciales
void setup(){
  Serial.begin(115200);
  pinMode(Pin_Led_Rojo, OUTPUT);
  pinMode(Pin_Led_Amarillo, OUTPUT);
  pinMode(Pin_Led_Verde, OUTPUT);

  //Conexión WIFI con Ubidots
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  //Recepcion de variables
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_1);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_2);

  //Condiciones inciales de Leds fisicamente y en Ubidots (botoneria)
  ubidots.add(VARIABLE_LABEL, 1);
  ubidots.add(VARIABLE_LABEL_1, 3);
  ubidots.add(VARIABLE_LABEL_2, 5);
  digitalWrite(Pin_Led_Rojo,LOW);
  digitalWrite(Pin_Led_Amarillo,LOW);
  digitalWrite(Pin_Led_Verde,LOW);

  //Configuracion rele y mensaje telegram
  pinMode(Pin_rele,OUTPUT);
  pinMode(Text_Telegram,OUTPUT);
  digitalWrite(Text_Telegram,LOW);
  timer=millis();   
}

//Funcion Loop
void loop(){
  if (!ubidots.connected()){
    ubidots.reconnect();    
  }
 
  //Se inicia la medicion de temperatura y campo magnetico
  Temp_Aux=millis();
  if (abs(Temp_Aux-timer) > PUBLISH_FREQUENCY){
    Hall = analogRead(A0);
    Lm35 = analogRead(39); 
    tiempo_actual=millis(); 
    delta_tiempo= tiempo_actual-tiempo_anterior;
    voltA=(3.3*Hall)/4095;
    voltA=voltA+1;
    voltB=(3.3*Lm35)/4095;
    gauss=666.67*voltA-1666.675;
    teslas=gauss*0.0001;
    if(gauss > 600){
      digitalWrite(Text_Telegram,HIGH);
    }else{
      digitalWrite(Text_Telegram,LOW);
    }
    
    //Calcular Tiempo
    Serial.println(delta_tiempo);
    if(delta_tiempo > 1000){
      contador+=1;
      Serial.println(contador);
      tiempo_anterior=tiempo_actual;
    } 
    
    //Condiciones de control campo magnetico
    if(gauss<-40 || gauss>40){
      if(contador<10){
        digitalWrite(Pin_rele, LOW);
      }else{
        digitalWrite(Pin_rele, HIGH);   
      }
      Aux=0; 
    }else{
      if(contador>20){
        digitalWrite(Pin_rele, LOW);
        contador=0;
      }
      Aux=voltB*100+5;
    }
    
    //Definicion del valor de Temperatura que será enviado por la variable 3, a la plataforma Ubidots
    ubidots.add(VARIABLE_LABEL_3, gauss);
    
    //Definicion del valor de Humedad que será enviado por la variable 4, a la plataforma Ubidots
    ubidots.add(VARIABLE_LABEL_4, Aux);
    ubidots.add(VARIABLE_LABEL_5, contador);

    //Publicación de los datos en el dispositivo definido
    ubidots.publish(DEVICE_LABEL);
    
    //Imprimir en el serial para comprobacion de envio
    Serial.println("Enviando los datos a Ubidots: ");
    Serial.println("Campo: " + String(Hall));
    Serial.println("Temperatura: " + String(Aux));
    Serial.println("Tiempo: " + String(contador));
    Serial.println("-----------------------------------------");
    timer = millis();
  }
  
  delay(100); 
  ubidots.loop();
}
