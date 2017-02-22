#include <SPI.h>
#include <WiFi101.h>



// -------- Definitions ------  //
#define waitForOrder 0
#define waitToStart 1
#define waitForWater 2
#define waitForPump 3
#define addPowder 4
#define mix 5
#define dispense 6
#define fillingLeft 7
#define fillingRight 8
#define checkOrder 9
#define orderFilled 10
#define drinksTaken 11

#define hotWaterOff 0
#define heatingWater 1
#define waterHot 2

#define hotWaterPotPin 7
#define mixerPin 8
#define leftValve 9
#define rightValve 10

// Global Variables //

// -- Wifi Variables -- //
char ssid[] = "XXXXXXXXXXXX";      //  your network SSID (name)
char pass[] = "XXXXXXXXXXXX";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);  // "Creates a server that listens for incoming connections on the specified port" - arduino website
int bodyNum = 0; // which body to send to client.

// Global Variables - Order
bool orderRec = false; // True when order placed via web interface
int orderCup = 0; // The number of cups ordered
int orderTime = 0; // The time in min from now the Hot Cocoa is desired

// Global Variables - Finite State Machines
int mainState = waitForOrder;
int waterHeaterState = hotWaterOff; 
unsigned long msCount = 0; // going to us millis() functionality
// 24 hrs in ms is 8.6 million, this is plenty big enough for this use. (Remove note later?)
bool hotWaterWanted = false; //If hot water is wanted
bool hotWaterReady = false;  //If the water is ready
float waterTemp = 0; // Temp of the water in degrees F
bool pumpWater = false; // If water should be pumped
bool waterPumped = false; // If water has been pumped
unsigned long mixTime = 60; // The desired mix time in seconds
bool cupLeftFull = false; // false = empty , true = full 
bool cupRightFull = false; // false = empty , true = full 
unsigned long fillTime = 10; // The time in seconds to dispense the cocoa (time valve is open) 
bool orderReady = false; // If the order is ready for pickup
bool buttonPress = false; // The button pressed to signal drinks taken (TODO change name)
bool refillButton = false; // The button pressed to signal a refill is wanted
unsigned long waitTime = 0; // ------------------------------- TODO ------------ need to address wait time
WiFiClient client = false; // making client a global variable 

void setup() {

// Wifi Setup //
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin(); // start the web server on port 80 (defined above around line 36)
  

  
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
              orderTime = 1; // hard coded now for example - TODO make it a variable
              orderCup = 2;   // hard coded now for example - TODO make it a variable
           }
           if (currentLine.endsWith("GET /status")) {
             bodyNum = 2;
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
      if((millis() - msCount) >= (waitTime * 1000)){  // TODO diagram typo msCounter -> msCount
        mainState = waitForWater;
        hotWaterWanted = true;
      }
      break;
    case waitForWater:
      if(hotWaterReady == true){
        mainState = waitForPump;
        pumpWater = true;
      }
      break;
    case waitForPump:
      if(waterPumped == true){
        mainState = addPowder;
        digitalWrite(mixerPin, HIGH);
        dispensePowder();
      }
      break;
    case addPowder:
      if(true){
        mainState = mix;
        msCount = millis();
      }
      break;
    case mix:
      if((millis() - msCount) >= (mixTime * 1000)){ // (TODO *1000 on diagram)
        mainState = dispense;
        digitalWrite(mixerPin, LOW);
      }
      break;
    case dispense:
      if(cupLeftFull == false){
        mainState == fillingLeft;
        digitalWrite(leftValve, HIGH);
        msCount = millis();                   // (TODO add msCount = 0)
      } else if (cupRightFull == false) {
        mainState == fillingRight;
        digitalWrite(rightValve, HIGH);
        msCount = millis();                   // (TODO add msCount = 0)
      }
      break;
    case fillingLeft:
      if((millis() - msCount) >= fillTime){
        mainState = checkOrder;
        cupLeftFull = true;
      }
      break;      
    case fillingRight:
      if((millis() - msCount) >= fillTime){
        mainState = checkOrder;
        cupRightFull = true;
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
        mainState = waitForWater;
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
        mainState = waitForWater;
        orderCup == 1; // (TODO Add to diagram)
      }
      break;  
  }  // End Main FSM
  
// ------- Hot Water Finite State Machine --------- //
  switch (waterHeaterState)
    {
      case hotWaterOff:
        if(hotWaterWanted == true){
          digitalWrite(hotWaterPotPin,HIGH);
          waterHeaterState = heatingWater;
        }
        break;
      case heatingWater:
         waterTemp = getTemp(); // Update Water Temp 
        if(hotWaterWanted == false){
          waterHeaterState = hotWaterOff;
          digitalWrite(hotWaterPotPin,LOW);
        } else if(waterTemp >= 180){
          waterHeaterState = waterHot;
          digitalWrite(hotWaterPotPin,LOW);
          hotWaterReady = true;
        }
        break;
      case waterHot:
        waterTemp = getTemp(); //Update Water Temp
        if(waterTemp <= 170){
          //Same State
          digitalWrite(hotWaterPotPin,HIGH);
        } else if(waterTemp >= 180){
          //Same State
          digitalWrite(hotWaterPotPin,LOW);
        } else if(hotWaterWanted == false){
          waterHeaterState = hotWaterOff;
          digitalWrite(hotWaterPotPin,LOW);
          hotWaterReady = false;
        }
        break;
      } // End Hot Water FSM

} // End Loop

void dispensePowder(){

  // TODO
  
}


// This function clears the current order //
void clearOrder(){

  orderRec = false;
  orderCup = 0;
  orderTime = 0;
  
} // End clearOrder

float getTemp(){

// TODO

}// End getTemp

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









