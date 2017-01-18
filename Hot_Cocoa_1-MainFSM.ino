//Define the states of the machine
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

// Global Variables
int mainState = waitForOrder;
int waterHeaterState = hotWaterOff; 

// Global Variables - Order
bool orderRec = false; // True when order placed via web interface
int orderCup = 0; // The number of cups ordered

// Global Variables - Finite State Machines
int msCount = 0; // On MKR1000 int max is 2,147,483,647.  (NOT SO ON UNO and similar boards!)
// 24 hrs in ms is 8.6 million, this is plenty big enough for this use. (Remove note later?)
bool hotWaterWanted = false; //If hot water is wanted
bool hotWaterReady = false;  //If the water is ready
float waterTemp = 0; // Temp of the water in degrees F
bool pumpWater = false; // If water should be pumped
bool waterPumped = false; // If water has been pumped
int mixTime = 60; // The desired mix time in seconds
bool cupLeftFull = false; // false = empty , true = full 
bool cupRightFull = false; // false = empty , true = full 
int fillTime = 10; // The time in seconds to dispense the cocoa (time valve is open) 
bool orderReady = false; // If the order is ready for pickup
bool buttonPress = false; // The button pressed to signal drinks taken (TODO change name)
bool refillButton = false; // The button pressed to signal a refill is wanted
int waitTime = 0; // ------------------------------- TODO ------------ need to address wait time

void setup() {

  pinMode(hotWaterPotPin, OUTPUT);
  pinMode(mixerPin, OUTPUT);
  pinMode(leftValve, OUTPUT);
  pinMode(rightValve, OUTPUT);
}

void loop() {

// ------- Main Finite State Machine --------- //
  switch (mainState)
  {
    case waitForOrder:
      if(orderRec == true){
        mainState = waitToStart;
        msCount = 0;
      }
      break;
    case waitToStart:
      if(msCount >= (waitTime * 1000)){  // TODO diagram typo msCounter -> msCount
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
        msCount = 0;
      }
      break;
    case mix:
      if(msCount >= (mixTime * 1000)){ // (TODO *1000 on diagram)
        mainState = dispense;
        digitalWrite(mixerPin, LOW);
      }
      break;
    case dispense:
      if(cupLeftFull == false){
        mainState == fillingLeft;
        digitalWrite(leftValve, HIGH);
        msCount = 0;                   // (TODO add msCount = 0)
      } else if (cupRightFull == false) {
        mainState == fillingRight;
        digitalWrite(rightValve, HIGH);
        msCount = 0;                   // (TODO add msCount = 0)
      }
      break;
    case fillingLeft:
      if(msCount >= fillTime){
        mainState = checkOrder;
        cupLeftFull = true;
      }
      break;      
    case fillingRight:
      if(msCount >= fillTime){
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
  }


  
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
   
      }
    

} // End Loop

void dispensePowder(){

  // TODO
  
}

void clearOrder(){

  // TODO
  
}

float getTemp(){

// TODO

}// End getTemp

