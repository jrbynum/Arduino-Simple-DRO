

/*
________________________________________________________________
************Program Name:CALIPER_DATA_OUTPUT_05_20_2013
************By J. Poston:
**********Date Initiated:4-22-213
************Date Revised:5-20-2013
*/
int dataIn = 4;
int clockIn = 2;
const int buttonPin = 5;     // the number of the pushbutton pin
int buttonState =0;         // variable for reading the pushbutton status
const int i=10;  //Determines the DELAY Low and High for each led state
int isin=0; //inch=1?mm=0
int isfs=0; //
int index;
unsigned long xData,oData;//
long previousMillis = 0; // will store last time LED was updated
long interval = 500; // interval at which to blink (milliseconds)
long previousGetMillis = 0; //
long Timeout = 8; // 8ms
const int ledon=3; // verification for experimenting change
int m;
void setup()
{

 // initialize the pushbutton pin as an input:
 pinMode(buttonPin, INPUT);     
 digitalWrite(dataIn, 1);
 digitalWrite( clockIn, 1);
 pinMode(dataIn, INPUT); //DATA line goes to Arduino digital pin 4
 pinMode(clockIn, INPUT);
 Serial.begin(38400);
 delay(500);
 attachInterrupt(0,getBit,RISING); //CLOCK line goes to Arduino digital pin 2 
 index =0;
 xData=0;
 oData=999;

 Print_Header();//Runs the Subroutine
 pinMode(ledon,OUTPUT); //makes digital pin 3 and output
 Test_Light();  //Runs the subroutine that makes pin 3 blink and LED
}
void loop()
{
 buttonState = digitalRead(buttonPin);
 delay(5);
 //if (buttonState == HIGH) 
 //{     
   if ((index !=0) && (millis() - previousGetMillis > Timeout) ) {
     index=0;
     xData=0;
   };
   if (index >23) {
     if (oData !=xData) {
       if (isfs==1)
         Serial.print('-');
       if (isin==1){ // 
         xData *=5;
         Serial.print(xData/10000);
         Serial.print('.');
         if ((xData % 10000)<1000){
           if ((xData % 10000)<100){
             if ((xData % 10000)<10){
               Serial.print('0');
             };
             Serial.print('0');
           };
           Serial.print('0');
         };
         Serial.println(xData % 10000);
         delay(100); //keeps inches from spitting out lots of the same measures
       }
       else { // mm

         Serial.print(xData/100);
         Serial.print('.');
         if ((xData % 100)<10) //0
             Serial.print('0');
         Serial.println(xData % 100);
       };
     }; 
     oData =xData;
     index=0;
     delay(5);
     xData=0;
   };
   if (millis() - previousMillis > interval) {
     previousMillis = millis();
   }
 //}
}
void getBit(){
 previousGetMillis=millis();
 if(index < 20){
   if(digitalRead(dataIn)==1){
     xData|= 1<<index;

   };
 }
 else {

   if (index==20) 
     isfs=digitalRead(dataIn);
   if (index==23) //24 1=inch
     isin=digitalRead(dataIn);
 };
 index++;  
}
/*
^^^^^^^^^^^^^^^^^^^^^^^^Sub Routines^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/
void Print_Header()
{
 Serial.println("****************************************************************");
 Serial.println("     Program: Digital Caliper Data SPC  (Running)               ");
 Serial.println("****************************************************************");

}

void Test_Light()
{
 delay(100);
 for(m=0;m<20;m++)
 {

   digitalWrite(ledon,HIGH);
   delay(30);
   digitalWrite(ledon,LOW);
   delay(30);
 }

}

