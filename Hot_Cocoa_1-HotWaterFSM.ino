//Define the states of the machine
#define hotWaterOff 0
#define heatingWater 1
#define waterHot 2

#define hotWaterPotPin 7

// Global Variables
int waterHeaterState = hotWaterOff; 
bool hotWaterWanted = false; //If hot water is wanted
bool hotWaterReady = false;  //If the water is ready
float waterTemp = 0; // Temp of the water in degrees F


void setup() {

  pinMode(hotWaterPotPin, OUTPUT);

}

void loop() {
  
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

float getTemp(){

// TODO

}// End getTemp

