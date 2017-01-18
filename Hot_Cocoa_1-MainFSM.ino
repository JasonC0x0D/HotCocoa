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
#define orderReady 10
#define drinksTaken 11

#define hotWaterOff 0
#define heatingWater 1
#define waterHot 2

#define hotWaterPotPin 7
#define mixerPin 8

// Global Variables
int mainState = waitForOrder;
int waterHeaterState = hotWaterOff; 


int msCount = 0; // On MKR1000 int max is 2,147,483,647.  (NOT SO ON UNO and similar boards!)
// 24 hrs in ms is 8.6 million, this is plenty big enough for this use. (Remove note later?)
bool hotWaterWanted = false; //If hot water is wanted
bool hotWaterReady = false;  //If the water is ready
float waterTemp = 0; // Temp of the water in degrees F
bool pumpWater = false; // If water should be pumped
bool waterPumped = false; // If water has been pumped
int mixTime = 60; // The desired mix time in seconds
bool cupLeftFull = false; // false = empty , true = full (TODO change diagram)
bool cupRightFull = false; // false = empty , true = full (TODO change diagram)
int fillTime = 10; // The time in seconds to dispense the cocoa (time valve is open)
int orderCup = 0; // The number of cups ordered 
bool orderReady = false; // If the order is ready for pickup
bool buttonPress = false; // The button pressed to signal drinks taken (TODO change name)
bool refillButton = false; // The button pressed to signal a refill is wanted

void setup() {

  pinMode(hotWaterPotPin, OUTPUT);
  pinMode(mixerPin, OUTPUT);
}

void loop() {

// ------- Main Finite State Machine --------- //
  switch (mainState)
  {
    case waitForOrder:

      break;
    case waitToStart:

      break;
    case waitForWater:
      
      break;
    case waitForPump:

      break;
    case addPowder:

      break;
    case mix:

      break;
    case dispense:

      break;
    case fillingLeft:

      break;      
    case fillingRight:

      break;      
    case checkOrder:

      break;
    case orderReady:

      break;  
    case drinksTaken:

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

