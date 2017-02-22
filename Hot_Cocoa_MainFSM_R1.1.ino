#include <SPI.h>
#include <WiFi101.h>

// -------- Definitions ------  //
#define waitForOrder 0
#define waitToStart 1
#define heatingWater 2
#define pumpingWater 3
#define addingCocoa 4
#define mix 5
#define dispense 6
#define fillingLeft 7
#define fillingRight 8
#define checkOrder 9
#define orderFilled 10
#define drinksTaken 11

#define hotWaterPotPin 6
#define mixerPin 2
#define leftValve 4
#define rightValve 5
#define pumpPin 3

#define DISTANCE 5000
#define stepperDirection 8
#define stepperStep 9

// --------------- Global Variables ---------------------- //

// -- Wifi Variables -- //
char ssid[] = "XXXXXXXXXX";      //  your network SSID (name)
char pass[] = "XXXXXXXXXX";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);  // "Creates a server that listens for incoming connections on the specified port" - arduino website
int bodyNum = 0; // which body to send to client.
WiFiClient client = false; // making client a global variable

// -- Order Variables -- //
bool orderRec = false; // True when order placed via web interface
int orderCup = 2; // The number of cups ordered
int orderTime = 0; // The time in min from now the Hot Cocoa is desired

// -- Finite State Machine -- //
int mainState = waitToStart;
unsigned long msCount = 0; // going to us millis() functionality
// 24 hrs in ms is 8.6 million, this is plenty big enough for this use. (Remove note later?)
unsigned long mixTime = 2; // The desired mix time in seconds
bool cupLeftFull = false; // false = empty , true = full 
bool cupRightFull = false; // false = empty , true = full 
unsigned long fillTime = 20; // The time in seconds to dispense the hot cocoa (time valve is open) 
bool orderReady = false; // If the order is ready for pickup
bool buttonPress = false; // The button pressed to signal drinks taken (TODO change name)
bool refillButton = false; // The button pressed to signal a refill is wanted
unsigned long waitTime = 0; // ------------------------------- TODO ------------ need to address wait time
bool conversionStart = false; // tempSensor if the conversion has started yet. 
int heatingTime = (5*60); // Time to heat water in seconds
int waterPumpWaitTime = 5;// Time to pump in seconds

// -- Stepper Motor -- //
int StepCounter = 0;
int Stepping = false;
int Powder = 0;

void setup() {
// ----------------------- Serial Setup --------------------- //
  Serial.begin(9600);
  delay(3000);
  Serial.println("Serial Port Opened");
  
// ------------------------- Wifi Setup -------------------- //

// TODO static IP address... not currently working may need to update IDE? though unlikely.
// IPAddress desiredIP = (192, 168, 11, 23);
// WiFi.config(desiredIP);
  
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);    
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin(); // start the web server on port 80 (defined above around line 36)

  // print the MKR1000's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  
// ----------------------- Powder Delivery Setup -------------- //
  pinMode(stepperStep, OUTPUT);    
  pinMode(stepperDirection, OUTPUT);
  digitalWrite(stepperStep, LOW);
  digitalWrite(stepperDirection, LOW);

  
// Finite State Machine Setup //
  pinMode(hotWaterPotPin, OUTPUT);
  digitalWrite(hotWaterPotPin, LOW);
  pinMode(mixerPin, OUTPUT);
  digitalWrite(mixerPin, LOW);
  pinMode(leftValve, OUTPUT);
  digitalWrite(leftValve, LOW);
  pinMode(rightValve, OUTPUT);
  digitalWrite(rightValve, LOW);
  
}

void loop() {

  Serial.print("Current State: ");
  Serial.println(mainState);

// ------ Wifi Server ------- //

  client = server.available();  // listen for incoming clients
    if(client){
      String currentLine = "";    // String to hold incoming data from the client
      while(client.connected()){  // loop while the client is connected
        if(client.available()){   // if there are bytes to read from the client
          char c = client.read(); // read a byte
          if(c == '\n') {         // if the byte is a newline character
          
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
               // HTTP Response
               sendHeader();
               sendBody(bodyNum); // 1 is the initial body
               sendEnd();
               // break out of the while loop when done
               break;
             }
             else {      // if you got a newline, then clear currentLine:
               currentLine = "";
             }
           } // end if(c == '\n')
           else if (c != '\r') {    // if you got anything else but a carriage return character,
             currentLine += c;      // add it to the end of the currentLine
           }
           // Check the client request:
           if (currentLine.endsWith("GET /order")) {
              orderRec = true;               // order recived!
              orderTime = 0; // hard coded now for example - TODO make it a variable
              orderCup = 2;   // hard coded now for example - TODO make it a variable
           }
           if (currentLine.endsWith("GET /state")) {
             bodyNum = 1;
           }         
         }      
       }
    } // end if(client)
    // close the connection:
    client.stop();

// ------- Main Finite State Machine --------- //
  switch (mainState)
  {
    case waitForOrder:
      if(orderRec == true){
        mainState = waitToStart;
        msCount = millis();
        waitTime = (unsigned long)orderTime * 60 * 1000; // convert ordertime (min) to waitTime (ms) 
        // max is 4.2 billion = which means the max order time is about 49 days. (plenty!)
        // FYI (unsigned long) is used for type casting 
      }
      break;
    case waitToStart:
      if((millis() - msCount) >= (waitTime)){ 
        mainState = heatingWater;
        digitalWrite(hotWaterPotPin, HIGH);
        msCount = millis();
      }
      break;
    case heatingWater:
      if((millis() - msCount) >= (heatingTime * 1000)){
        mainState = pumpingWater;
        digitalWrite(hotWaterPotPin, LOW);
        digitalWrite(pumpPin, HIGH);
        msCount = millis();
      }
      break;
    case pumpingWater:
      if((millis() - msCount) >= (waterPumpWaitTime * 1000)){
        mainState = addingCocoa;
        digitalWrite(mixerPin, HIGH);
        digitalWrite(pumpPin,LOW);
        dispensePowder();
      }
      break;
    case addingCocoa:
      if(true){
        dispensePowder();
        mainState = mix;
        msCount = millis();
      }
      break;
    case mix:
      if((millis() - msCount) >= (mixTime * 1000)){
        mainState = dispense;
        digitalWrite(mixerPin, LOW);
      }
      break;
    case dispense:
      if(cupLeftFull == false){
        mainState = fillingLeft;
        digitalWrite(leftValve, HIGH);
        msCount = millis();                   // (TODO add msCount = 0)
      } else if (cupRightFull == false) {
        mainState = fillingRight;
        digitalWrite(rightValve, HIGH);
        msCount = millis();                   // (TODO add msCount = 0)
      } 
      break;
    case fillingLeft:
      if((millis() - msCount) >= fillTime * 1000){
        mainState = checkOrder;
        cupLeftFull = true;
        digitalWrite(leftValve, LOW);
      }
      break;      
    case fillingRight:
      if((millis() - msCount) >= fillTime * 1000){
        mainState = checkOrder;
        cupRightFull = true;
        digitalWrite(rightValve, LOW);
      }
      break;      
    case checkOrder:
      if(orderCup == 1){
        mainState = orderFilled;
        orderReady = true;
      } else if ( orderCup == 2 && cupRightFull == true){ // (TODO fix diagram cupRightFull!!!)
        mainState = orderFilled;
        orderReady = true;  
      } else if ( orderCup == 2 && cupRightFull == false){
        mainState = pumpingWater;
        msCount = millis();
        digitalWrite(pumpPin, HIGH);
      }
      break;
    case orderFilled:
      if(buttonPress == true){
        mainState = drinksTaken;
        clearOrder();
      }
      break;  
    case drinksTaken:
      if(refillButton == true){
        mainState = pumpingWater;
        orderCup = 1;
        digitalWrite(pumpPin, HIGH);
        msCount = millis();
      }
      break;  
  }  // End Main FSM

} // End Loop

void dispensePowder()
{
  digitalWrite(stepperDirection, LOW);     // Direction pin low to move "forward"
  static int msCurrent = 0;
  static int delayTime = 200; // delay time between steps in microseconds
                              // lower delay time = motor spinning faster
                              // however too fast and the motor can't keep up.
  
  for(int x= 1; x<12000; x++)  // With the current setup this dispenses about 1 Tablespoon of cocoa powder (12000 steps = 1 Tablespoon)
    {
    digitalWrite(stepperStep,HIGH); // Trigger one step forward
    msCurrent = micros();
    while(micros() <= (msCurrent+delayTime)){ // using this timing method instead of delay becuase it is faster
      // do nothing
    }
    digitalWrite(stepperStep,LOW); //Pull step pin low so it can be triggered again
    msCurrent = micros();
    while(micros() <= (msCurrent+delayTime)){
      // do nothing
    }
  }
  
} // End dispensePowder

void mixCocoa(bool mixerOn){
  if( mixerOn == true){
    // turn on the mixer
  } else if ( mixerOn == false){
    // turn of the mixer
  }
}


// This function clears the current order //
void clearOrder(){

  orderRec = false;
  orderCup = 0;
  orderTime = 0;
  
} // End clearOrder



// ----- WIFI Functions ------//
void sendHeader(){
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
            
} // End sendHeader

void sendBody(int num){
  switch(num){
    case 0: // Initial body
      client.print("Welcome to the Hot Cocoa 3000<br>");
      client.print("Click <a href=\"/order\">here</a> to order some hot cocoa<br>");
      client.print("Click <a href=\"/state\">here</a> to check the order status<br>");
      break;
    case 1: // State Check body
      client.print("Welcome to the Hot Cocoa 3000<br>");
      client.print("The machine is currently in state:");
      client.print(mainState);
      client.print("<br>");
      client.print("Click <a href=\"/state\">here</a> to check the order status<br>");
      break;  
  }
   

}  // End sendBody

void sendEnd(){
  client.println();
} // end sendEnd


