//Mesure sur Wattmètre Peacefair PZEM004Tv3.0 01/03/21
#include <PZEM004Tv30.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <string>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ThingSpeak.h>
#include <ESP8266HTTPClient.h>
//#include "SoftwareSerial.h"

//Cablage esp V3
//PZEM          NodeMcuV3
//5V            VU
//GND           G
//RX            D2/D5/D7
//TX            D1/D3/D6

//Variables
const char* ssid     ="XXX";//SSID du Wifi
const char* password = "xxx";//Mot de passe du Wifi
byte writeSuccess =0;

//Déclaration des variables
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
unsigned long minute_depuis_init = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 8000;
String weekDays[7]={"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
String months[12]={"Janvier", "Février", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Décembre"};
byte monthDay=0;
byte currentMonth=0;
byte jour_sem_courant=0;
byte jour_init = 0;
byte mois_init = 0;
int an_init = 0;
byte heure_init = 0;
byte minute_init = 0;
byte second_init = 0;
// ThingSpeak information
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 1299910;
char* readAPIKey = "XXXX";
char* writeAPIKey = "XXXXX";
float Tension =0;
float Intensite =0;
float Frequence =0;
float Fp =0;
float Puissance =0;
float MaxPuissance =0;
float MinPuissance =50000;
float energie =0;
float prix_base = 172.58;
float prix_jour = 185.85;
float prix_nuit = 142.0;
float prix_abon_base =22.95;
float prix_abon_HC =35.51;
float cout_journee =0;
float cout_nuitee =0;
float energie_jour =0;
float energie_nuit =0;
float cout_base =0;
float cout_JN =0;
float cumul_nuit =0; // Cumul pour gérer l'événtuelle succession de périodes nuit
float debut_nuit =0;
bool periode_nuit =0;
String periode = "Jour";
unsigned int conso[10] = {0,0,0,0,0,0,0,0,0,0};
float cout_jour[10] = {0,0,0,0,0,0,0,0,0,0};
float cout_nuit[10] = {0,0,0,0,0,0,0,0,0,0};
byte jour_sem[10] ={0,0,0,0,0,0,0,0,0,0};
//PZEM004T RX TX
PZEM004Tv30 pzem1(D1, D2);//(RX,TX) connection au PZEM 1

ESP8266WebServer server(80);         // On instancie un serveur qui ecoute sur le port 80
//Déclaration client
WiFiClient espClient; //Déclaration client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
AlarmId id;

String getPage(){
  String page = "<html lang=fr-FR><head><meta http-equiv='refresh' content='2'/><meta charset='utf-8'>";
  page += "<title>PZEM Comptage énergie - www.projetsdiy.fr</title>";
  page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>";
  page += "</head><body><h1>ESP8266-10 PZEM004 v.1</h1>";
  page += "<ul><li>Heure : ";
  page += timeClient.getFormattedTime();
  page += "  /  ";
  page += weekDays[timeClient.getDay()];page += monthDay; page += " "; page += months[currentMonth-1];
  page += "</ul><h3>Mesures :</h3>";
  page += "</ul></li><li>Tension : ";
  page += Tension;   
  page += " V</li>";
  page += "<li>Intensité : ";
  page += Intensite;
  page += " A </li>";
  page += "<li>Fréquence : ";
  page += Frequence;
  page += " Hz </li>";
  page += "<li>Facteur puissance : ";
  page += Fp;
  page += "</li>";
  page += "<li>Puissance : ";
  page += Puissance;
  page += " W </li>";
  page += "<li>Période tarif : ";
  page += periode;
  page += "</li>";
  page += "<li>Energie : ";
  page += energie;
  page += " kWh</li>";
  page += "<br><li>Energie jour : "; page += energie_jour; page += " kWh  / " ;page += cout_journee; page += " €</li>";
  page += "<li>Energie nuit : "; page += energie_nuit; page += " kWh  / " ;page += cout_nuitee; page += " €</li>";
   page += "<li>Maxi P : ";
  page += (unsigned int)MaxPuissance;
  page += " W </li>";
  page += "<li>Mini P  : ";
  page += (unsigned int)MinPuissance;
  page += " W</li></ul>";
  page += "<h3>Historique : Conso. / Jour - Nuit</h3> <li>";
  page += weekDays[jour_sem[0]]; page += " : "; page += conso[0]; page += " kWh / "; page += cout_jour[0]; page += " - "; page += cout_nuit[0]; page += "  €</li><li>";
  page += weekDays[jour_sem[1]]; page += " : "; page += conso[1]; page += " kWh / "; page += cout_jour[1]; page += " - "; page += cout_nuit[1]; page += "  €</li><li>";
  page += weekDays[jour_sem[2]]; page += " : "; page += conso[2]; page += " kWh / "; page += cout_jour[2]; page += " - "; page += cout_nuit[2]; page += "  €</li><li>";
  page += weekDays[jour_sem[3]]; page += " : "; page += conso[3]; page += " kWh / "; page += cout_jour[3]; page += " - "; page += cout_nuit[3]; page += "  €</li><li>";
  page += weekDays[jour_sem[4]]; page += " : "; page += conso[4]; page += " kWh / "; page += cout_jour[4]; page += " - "; page += cout_nuit[4]; page += "  €</li><li>";
  page += weekDays[jour_sem[5]]; page += " : "; page += conso[5]; page += " kWh / "; page += cout_jour[5]; page += " - "; page += cout_nuit[5]; page += "  €</li><li>";
  page += weekDays[jour_sem[6]]; page += " : "; page += conso[6]; page += " kWh / "; page += cout_jour[6]; page += " - "; page += cout_nuit[6]; page += "  €</li><li>";
  page += weekDays[jour_sem[7]]; page += " : "; page += conso[7]; page += " kWh / "; page += cout_jour[7]; page += " - "; page += cout_nuit[7]; page += "  €</li><li>";
  page += weekDays[jour_sem[8]]; page += " : "; page += conso[8]; page += " kWh / "; page += cout_jour[8]; page += " - "; page += cout_nuit[8]; page += "  €</li><li>";
  page += weekDays[jour_sem[9]]; page += " : "; page += conso[9]; page += " kWh / "; page += cout_jour[9]; page += " - "; page += cout_nuit[9]; page += "  €</li></li></ul>";
  page += "<br><li>Coût en base depuis Init : "; page += cout_base; page += "  €</li>";
  page += "<li>Coût en J/N depuis Init : "; page += cout_JN; page += "  €</li>";
  page += "<h3>Commandes :</h3>";
  page += "<form action='/' method='POST'>";
  page += "<ul><li>Compteur Max/Min : ";
  page += "<INPUT type='submit' name='RESETM' value='Reset'></li></ul>";
  page += "<ul><li>Compteur Journalier : ";
  page += "<INPUT type='submit' name='RESETJ' value='Reset'></li></ul>";
  page += "<h3>Paramètres :</h3>";
  page += "<li>Kwh Tjour : ";
  page += "<INPUT type='submit' name='Moins10jour' value='-10'> ";
  page += " <INPUT type='submit' name='Moinsjour' value='-1 '>     ";
  page += prix_jour;   
  page += " m€  <INPUT type='submit' name='Plusjour' value='+1 '> ";
  page += " <INPUT type='submit' name='Plus10jour' value='+10'> </li></ul>";
  page += "<li>Kwh Tnuit : ";
  page += "<INPUT type='submit' name='Moins10nuit' value='-10'> ";
  page += " <INPUT type='submit' name='Moinsnuit' value='-1 '>     ";
  page += prix_nuit;   
  page += " m€  <INPUT type='submit' name='Plusnuit' value='+1 '> ";
  page += " <INPUT type='submit' name='Plus10nuit' value='+10'> </li></ul>";
  page += "<li>Kwh Base : ";
  page += "<INPUT type='submit' name='Moins10base' value='-10'> ";
  page += " <INPUT type='submit' name='Moinsbase' value='-1 '>     ";
  page += prix_base;   
  page += " m€  <INPUT type='submit' name='Plusbase' value='+1 '> ";
  page += " <INPUT type='submit' name='Plus10base' value='+10'> </li></ul>";
  page += "<br>Initialisation : "; page += jour_init; page += "/"; page += mois_init; page += "/"; page += an_init; 
  page += " - "; page += heure_init; page += ":"; page += minute_init; 
  page += "<br><br><p><a hrf='https://www.projetsdiy.fr'>www.projetsdiy.fr</p>";
  page += "</body></html>";
  return page;
}
void handleRoot(){ 
  if ( server.hasArg("RESETM") ) {
    MaxPuissance =0 ;
    MinPuissance = 50000 ; 
     } 
    else if ( server.hasArg("Moinsjour") ) {
    prix_jour =prix_jour - 0.01;
    } 
    else if ( server.hasArg("Plusjour") ) {
    prix_jour =prix_jour + 0.01;
    }
    else if ( server.hasArg("Moins10jour") ) {
    prix_jour =prix_jour - 0.1;
    } 
    else if ( server.hasArg("Plus10jour") ) {
    prix_jour =prix_jour + 0.1;
    }      
    else if ( server.hasArg("Moinsnuit") ) {
    prix_nuit =prix_nuit - 0.01;
    } 
    else if ( server.hasArg("Plusnuit") ) {
    prix_nuit =prix_nuit + 0.01;
    }
    else if ( server.hasArg("Moins10nuit") ) {
    prix_nuit =prix_nuit - 0.1;
    } 
    else if ( server.hasArg("Plus10nuit") ) {
    prix_nuit =prix_nuit + 0.1;
    }
        else if ( server.hasArg("Moinsbase") ) {
    prix_base =prix_base - 0.01;
    } 
    else if ( server.hasArg("Plusbase") ) {
    prix_base =prix_base + 0.01;
    }
    else if ( server.hasArg("Moins10base") ) {
    prix_base =prix_base - 0.1;
    } 
    else if ( server.hasArg("Plus10base") ) {
    prix_base =prix_base + 0.1;
    }                            
   else if ( server.hasArg("RESETJ") ) Raz_energy() ; 
server.send ( 200, "text/html", getPage() );
}  
void WeeklyAlarm() {
  Serial.println("Alarm: - its Monday Morning");
}
void ExplicitAlarm() {
  Serial.println("Alarm: - this triggers only at the given date and time");
}
void write_tspeak() {
  
   if(WiFi.status()== WL_CONNECTED){
      bool begin (espClient); // defaults to ThingSpeak.com : initialise les paramètres réseau
      ThingSpeak.begin(espClient);
      // Use this function if you want to write multiple fields simultaneously.
//      ThingSpeak.setField( 1, energie );
//      ThingSpeak.setField( 2, energie_ph2 );
//      ThingSpeak.setField( 3, energie_ph3 );
//      ThingSpeak.setField( 4, energie_Totale );
//      ThingSpeak.setField( 5, MaxPuissance );
//      ThingSpeak.setField( 6, MaxPuissance_ph2 );
//      ThingSpeak.setField( 7, MaxPuissance_ph3 );
//      ThingSpeak.setField( 8, MaxPuissance_Totale );
//      int writeSuccess = ThingSpeak.writeFields( channelID, writeAPIKey );
      Serial.print("Write response: ");
      Serial.println(writeSuccess);// if(writeSuccess == 200)
    }
    else {
      Serial.println("WiFi Disconnected");
      WiFi.reconnect();
    }
}
void Repeats60() {
  Serial.println("60 second timer");
 // ++ energie;
 // energie=energie + 1.26;
 ++ minute_depuis_init;
cout_base =(prix_abon_base/30) * (1+ int(minute_depuis_init/1440))+(energie*prix_base/1000);
cout_JN =(prix_abon_HC/30) * (1+ int(minute_depuis_init/1440))+(energie_jour *prix_jour/1000)+(energie_nuit *prix_nuit/1000); 
}

void OnceOnly() {
  Serial.println("This timer only triggers once, stop the 2 second timer");
  // use Alarm.free() to disable a timer and recycle its memory.
  Alarm.free(id);
  // optional, but safest to "forget" the ID after memory recycled
  id = dtINVALID_ALARM_ID;
  // you can also use Alarm.disable() to turn the timer off, but keep
  // it in memory, to turn back on later with Alarm.enable().
}
void heure_ntp() {
  Serial.println("Timer NTP");
   // Met à jour l'heure 
  // Démarrage du client NTP - Start NTP client
  if(WiFi.status()== WL_CONNECTED){
    timeClient.begin();
    timeClient.setTimeOffset(3600);
    while(!timeClient.update()) {
    timeClient.forceUpdate(); // Récupère l'heure sur NTP
    }
    Serial.println(timeClient.getFormattedTime());
    unsigned long epochTime = timeClient.getEpochTime();//  Défini la variable pour récupérer la date
    Serial.print("Epoch Time: (depuis 1/1/1900 )" );
    Serial.println(epochTime);
    //Get a time structure 
    struct tm *ptm = gmtime ((time_t *)&epochTime); // pour extraire la date
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentSecond = timeClient.getSeconds();
    currentMonth = ptm->tm_mon+1;
    int currentYear = ptm->tm_year+1900;
    monthDay = ptm->tm_mday;
    int ete = ptm->tm_isdst;
    int jour_sem = ptm->tm_wday;
    Serial.print("Hour: ");
    Serial.println(currentHour); 
    Serial.print("Year: ");
    Serial.println(currentYear);
    Serial.print("Jour semaine: ");
    Serial.println(jour_sem);
    setTime(currentHour-ete,currentMinute,currentSecond,monthDay,currentMonth,currentYear-2000); // set time to NTP
      }
    else {
      Serial.println("WiFi Disconnected pas de mise à l'heure");
      WiFi.reconnect();  
    }
}
void Debut_tarif_nuit(){
  periode = "Nuit";
  periode_nuit =1;
  cumul_nuit = energie_nuit;
  debut_nuit= energie; 
}
void Fin_tarif_nuit(){
periode = "Jour";
 periode_nuit =0; 
}
void Raz_energy () {
//write_tspeak();
//  Mise à jour du tableau historique jour et conso.
  jour_sem_courant =timeClient.getDay()+1;
  for (int jour =9; jour>0 ; --jour){
     conso[jour]=conso[jour-1];
     cout_jour[jour]=cout_jour[jour-1]; 
     cout_nuit[jour]=cout_nuit[jour-1];
     jour_sem[jour]= (jour_sem_courant +13-jour)%7;
  }
  jour_sem[0]= (jour_sem_courant-1)%7;
  conso [0] = energie;
  cout_jour [0] = cout_journee;
  cout_nuit [0] = cout_nuitee;
// Reset des compteurs
  pzem1.resetEnergy();
  // Reset des seuils Maxi
  MaxPuissance=0;
  MinPuissance=50000;
  debut_nuit = 0;
  energie_jour =0;
  energie_nuit=0;
  cumul_nuit=0;

  
  Serial.println("RaZ d(u)(es) compteur(s) énergie(s)");
}
void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  printDigits(day());
  printDigits(month());
  printDigits(year());
  Serial.println();
}
void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");
  delay(10);
  currentTime = millis();
  previousTime = currentTime;
  WiFi.begin(ssid, password);
  Serial.println("");
  // on attend d'etre connecte au WiFi avant de continuer
while((WiFi.status() != WL_CONNECTED) && (currentTime - previousTime <= timeoutTime)) {
    currentTime = millis();
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status()== WL_CONNECTED){
  // on affiche l'adresse IP attribuee pour le serveur DNS
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
// Met à jour l'heure 
  heure_ntp();
//Défini la date initalisation du système
  unsigned long epochTime = timeClient.getEpochTime();//  Défini la variable pour récupérer la date
  //Get a time structure 
  struct tm *ptm = gmtime ((time_t *)&epochTime); // pour extraire la date
  mois_init = ptm->tm_mon+1;
  an_init = ptm->tm_year+1900;
  jour_init = ptm->tm_mday;
  heure_init = timeClient.getHours();
  minute_init = timeClient.getMinutes();
  second_init = timeClient.getSeconds();
//Mise à jour tableau date historique
  jour_sem_courant =timeClient.getDay();
  //Serial.println ( jour_sem_courant );
  for (int jour =9; jour>=0 ; --jour){
  //Serial.println(jour);
  jour_sem[jour]= (jour_sem_courant +13-jour)%7;
  //Serial.println(jour_sem[jour]);
  }
// initialisation de l'OTA
// No authentication by default

// ArduinoOTA.setHostname("ESP8266-");
// ArduinoOTA.setPassword((const char *)"MotDePasseAuthentification");
// ArduinoOTA.onStart([]() {
// Serial.println("Start");
//  });
//  ArduinoOTA.onEnd([]() {
//    Serial.println("\nEnd");
//  });
//  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//  });
//  ArduinoOTA.onError([](ota_error_t error) {
//    Serial.printf("Error[%u]: ", error);
//    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//    else if (error == OTA_END_ERROR) Serial.println("End Failed");
//  });
//  ArduinoOTA.begin();
  Serial.println("Ready");

  Serial.print("IP address: ");

  Serial.println(WiFi.localIP());
  // Initialisation serveur
  
  // On branche la fonction qui gère la premiere page
  server.on ( "/", handleRoot );
  server.begin();
  Serial.println ( "HTTP server started" );
  }
  else {
  setTime(12,0,0,1,1,20); // set time to Saturday 12:00:00am Jan 1 2020
  }
  // create the alarms, to trigger at specific times
  Alarm.alarmRepeat(4,30,0, heure_ntp);  // 4:30am every day
  Alarm.alarmRepeat(6,26,0,Fin_tarif_nuit);
  Alarm.alarmRepeat(22,26,0,Debut_tarif_nuit);
  Alarm.alarmRepeat(23,59,50,Raz_energy);  // 23:59am every day
  //Alarm.alarmRepeat(dowSaturday,8,30,30,WeeklyAlarm);  // 8:30:30 every Saturday
  // create timers, to trigger relative to when they're created
  
 //Alarm.timerRepeat(120, write_tspeak);           // timer for every 120 seconds écriture cloud
 id = Alarm.timerRepeat(60, Repeats60);      // timer for every 60 seconds
 // Alarm.timerOnce(10, OnceOnly);            // called once after 10 seconds
Serial.println("Ending setup...");
}

void loop() {
//ArduinoOTA.handle();  
// digitalClockDisplay(); // affiche l'heure sur moniteur série
 Alarm.delay(50); // wait   between clock display
  //Temps d'attente avant nouvelles mesures
  delay(50);
  Tension = pzem1.voltage();
  Intensite = pzem1.current();
  Frequence = pzem1.frequency();
  Fp = pzem1.pf();
  Puissance = pzem1.power();
 // energie = pzem1.energy();
  MaxPuissance = max(MaxPuissance,Puissance);
  MinPuissance = min(MinPuissance,Puissance);
  if (periode_nuit){
    energie_nuit = cumul_nuit + energie - debut_nuit;
    cout_nuitee = energie_nuit * prix_nuit/1000;
    cout_journee = energie_jour * prix_jour/1000;
  }
    else {
    energie_jour = energie-energie_nuit;
    cout_journee = energie_jour * prix_jour/1000;
    cout_nuitee = energie_nuit * prix_nuit/1000;
  }
  delay(200);

 
 
  server.handleClient(); // a chaque iteration, la fonction handleClient traite les requetes 

}
