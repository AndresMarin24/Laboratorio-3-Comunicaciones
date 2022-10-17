
//Se incluye la libreria de Telegram
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//Se define el Token de Telegram a usar junto con el chat ID
#define BOT_TOKEN "5711333129:AAFkwcrQmq6d7QELT_kAQpUNt0wmr_YC3c0"
#define CHAT_ID "5649229085"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

//Se define la Red wifi que se usara, tanto el SSID o nombre de la red como la contraseña
#define WIFI_SSID "Sara"
#define WIFI_PASSWORD "hellomoto"

//Pin receptor de la señal para envio de alerta a Telegram
const int Pin_alerta = 4;
//Seteo de configuracion y condiciones iniciales
void setup() {
  Serial.begin(115200);
  Serial.println();

  //Iniciar conexion a WIFI
  Serial.print("Conectando al WIFI: ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWIFI conectado. Dirección IP: ");
  Serial.println(WiFi.localIP());

  //Conexión con Telegram
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600){
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  //Recepcion de señal para envio de mensaje
  pinMode(Pin_alerta,INPUT);
}

//Funcion Loop
void loop() {
  if(digitalRead(Pin_alerta) == HIGH){
    bot.sendMessage(CHAT_ID, "¡ALERTA!, campo electromagnetico elevado, por favor elejese de la zona. CORRIENDO", "");
  }
}
