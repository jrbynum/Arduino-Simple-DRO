#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
  ________________________________________________________________
************Program Name:TWO_CHANNEL_DRO using cheap calipers
************By Rusty Bynum
************Date Initiated: 12-21-2016 - by Rusty Bynum
  Using the two available interrupts on the UNO
  TODO*****************************
   Add LCD display to display DSO's for X and Y axis
   Fix Hardware inverting error - see comments below.
  Committ to GitHub
*******************12-23-2016*******************************  
*Added LCD Stuff it all works on the bench will test it on the lathe
*once its working need to remove the serial port stuff
*Comitted update to github
*
*/

//************************************************************************
//X-Axis Stuff
//************************************************************************
int x_dataIn = 4;
int x_clockIn = 2;    //INT0
int x_isin = 0; //inch=1?mm=0
int x_isfs = 0; //negative number
int x_index;  //index to count bits
unsigned long xData, x_oData; //x axis data, old x axis data
long x_previousGetMillis = 0; //
long x_Timeout = 16; // 8ms

//************************************************************************
//Y-Axis Stuff
//************************************************************************
int y_dataIn = 7;
int y_clockIn = 3;    //INT1
int y_isin = 0; //inch=1?mm=0
int y_isfs = 0; //negative number
int y_index;  //index to count bits
unsigned long yData, y_oData; //y axis data, old y axis data
long y_previousGetMillis = 0; //
long y_Timeout = 16; // 8ms

//***********************************************
//LED Stuff
//***********************************************
long interval = 500; // interval at which to blink (milliseconds)
long previousMillis = 0; // will store last time LED was updated
const int ledon = 6; // verification for experimenting change
int m;
boolean toggle = true;

//**********************************************************************
//LCD Stuff
//
//Using cheap I2C White on Blue 16x2 LCD
//********************************************************************

LiquidCrystal_I2C lcd1(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


void setup()
{
  //***********************************************
  //X Axis pins Init
  //***********************************************
  digitalWrite(x_dataIn, 1);
  digitalWrite( x_clockIn, 1);
  pinMode(x_dataIn, INPUT); //DATA line goes to Arduino digital pin 4
  pinMode(x_clockIn, INPUT);

  //***********************************************
  //Y Axis pins Init
  //***********************************************
  digitalWrite(y_dataIn, 1);
  digitalWrite( y_clockIn, 1);
  pinMode(y_dataIn, INPUT); //DATA line goes to Arduino digital pin 7
  pinMode(y_clockIn, INPUT);

  //*******************
  //init Serial monitor
  //*******************
  Serial.begin(38400);
  delay(500);

  //***************
  //init interrupts
  //***************
  attachInterrupt(digitalPinToInterrupt(x_clockIn), x_getBit, RISING); //CLOCK line goes to Arduino digital pin 2
  //************************TODO*****************************************************
  //Pin 10 and 11 on the LM339 need to be swapped, inverting and non-inverting inputs were reversed... so... changed the interrupt to trigger on falling instead of rising and now its fixed in software for now
  attachInterrupt(digitalPinToInterrupt(y_clockIn), y_getBit, FALLING); //CLOCK line goes to Arduino digital pin 3

  //**************
  //init variables
  //**************
  x_index = 0;
  xData = 0;
  x_oData = 999;

  y_index = 0;
  yData = 0;
  y_oData = 999;

  Print_Header();//Runs the Subroutine
  //init LED pin
  pinMode(ledon, OUTPUT); //makes digital pin 3 and output
  //blink the led to indicate we are running
  Test_Light();  //Runs the subroutine that makes pin 3 blink and LED

  //************LCD***************
  lcd1.begin(16, 2);
  // Print a message on the LCD.
  lcd1.backlight();
  lcd1.clear();
  lcd1.setCursor(0, 0);
  lcd1.print("Arduino DRO");
  lcd1.setCursor(0, 1);
  lcd1.print("      By Rusty");
  delay(2000);
}

void loop()
{
  delay(5);
  Check_Xaxis();
  Check_Yaxis();
  //toggle LED to show we are alive!
  HeartBeat();

}//end loop

//********************************
//ISR 0 - rising edge of the clock
//********************************
void x_getBit()
{
  x_previousGetMillis = millis();
  if (x_index < 20)
  {
    if (digitalRead(x_dataIn) == 1)
    {
      xData |= 1 << x_index;
    }
  }
  else
  {
    if (x_index == 20)
      x_isfs = digitalRead(x_dataIn);
    if (x_index == 23) //24 1=inch
      x_isin = digitalRead(x_dataIn);
  }
  x_index++;
}

//********************************
//ISR 1 - rising edge of the clock
//********************************
void y_getBit()
{
  y_previousGetMillis = millis();
  if (y_index < 20)
  {
    if (digitalRead(y_dataIn) == 1)
    {
      yData |= 1 << y_index;
    }
  }
  else
  {
    if (y_index == 20)
      y_isfs = digitalRead(y_dataIn);
    if (y_index == 23) //24 1=inch
      y_isin = digitalRead(y_dataIn);
  }
  y_index++;
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
  for (m = 0; m < 20; m++)
  {
    digitalWrite(ledon, HIGH);
    delay(30);
    digitalWrite(ledon, LOW);
    delay(30);
  }
}

//******************************
//Routine to show we are running
//******************************
void HeartBeat()
{
  if (millis() - previousMillis > interval)
  {
    previousMillis = millis();
    if (toggle)
    {
      digitalWrite(ledon, HIGH);   // set the LED on
      toggle = !toggle;
    }
    else
    {
      digitalWrite(ledon, LOW);    // set the LED off
      toggle = !toggle;
    }   //digitalWrite(ledon,!digitalRead(ledon));
  }
}

//*********************************************************************
//Check X-AXIS Routine
//*********************************************************************
void Check_Xaxis()
{
  int sign = 1;
  float result;

  //check for timeout
  //index gets incremented in the interrupt routine
  //if its incremented but a timeout occurs then not valid data
  //so reset the index and update the timeout
  if ((x_index != 0) && (millis() - x_previousGetMillis > x_Timeout) )
  {
    x_index = 0;
    xData = 0;
    return;
  }

  //we have a reading so process and display it
  //index is correct with the correct number of bits so process it
  if (x_index > 23)
  {
    if (x_oData != xData)
    {
      Serial.print("X AXIS: ");

      if (x_isfs == 1) //check for negative number
      {
        Serial.print('-');
        sign = -1;
      }

      if (x_isin == 1)
      { //Inches
        xData *= 5;
        Serial.print(xData / 10000);
        Serial.print('.');
        if ((xData % 10000) < 1000)
        {
          if ((xData % 10000) < 100)
          {
            if ((xData % 10000) < 10)
            {
              Serial.print('0');
            }
            Serial.print('0');
          }
          Serial.print('0');
        }
        Serial.println(xData % 10000);

        //*************************************
        // Print to LCD
        //*************************************
        result = ((float)xData / 10000.00) * sign;
       // lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("X AXIS: ");
        lcd1.print(result);
        lcd1.print("   ");

        
        delay(100); //keeps inches from spitting out lots of the same measures
      }
      else
      { // mm
        Serial.print(xData / 100);
        Serial.print('.');
        if ((xData % 100) < 10) //0
          Serial.print('0');

        Serial.println(xData % 100);

        //*************************************
        // Print to LCD
        //*************************************
        result = ((float)xData / 100.00) * sign;
       // lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("X AXIS: ");
        lcd1.print(result);
        lcd1.print("   ");
      }
    }
    //set the old data with the new to check if its been updated
    x_oData = xData;
    x_index = 0; //reset the index
    delay(5); //a delay?
    xData = 0; //reset the data
  }
}

//*********************************************************************
//Check Y-AXIS Routine
//*********************************************************************
void Check_Yaxis()
{

  int sign = 1;
  float result;
  //check for timeout
  //index gets incremented in the interrupt routine
  //if its incremented but a timeout occurs then not valid data
  //so reset the index and update the timeout
  if ((y_index != 0) && (millis() - y_previousGetMillis > y_Timeout) )
  {
    y_index = 0;
    yData = 0;
    return;
  }

  //we have a reading so process and display it
  //index is correct with the correct number of bits so process it
  if (y_index > 23)
  {
    if (y_oData != yData)
    {
      Serial.print("Y AXIS: ");

      if (y_isfs == 1) //check for negative number
      {
        Serial.print('-');
        sign = -1;
      }
      if (y_isin == 1)
      { //Inches
        yData *= 5;
        Serial.print(yData / 10000);
        Serial.print('.');
        if ((yData % 10000) < 1000)
        {
          if ((yData % 10000) < 100)
          {
            if ((yData % 10000) < 10)
            {
              Serial.print('0');
            }
            Serial.print('0');
          }
          Serial.print('0');
        }
        Serial.println(yData % 10000);

        //*************************************
        // Print to LCD
        //*************************************
        result = ((float)yData / 10000.00) * sign;
        //lcd1.clear();
        lcd1.setCursor(0, 1);
        lcd1.print("Y AXIS: ");
        lcd1.print(result);
        lcd1.print("   ");

        
        delay(100); //keeps inches from spitting out lots of the same measures
      }
      else
      { // mm
        Serial.print(yData / 100);
        Serial.print('.');
        if ((yData % 100) < 10) //0
          Serial.print('0');

        Serial.println(yData % 100);

        //*************************************
        // Print to LCD
        //*************************************
        result = ((float)yData / 100.00) * sign;
        //lcd1.clear();
        lcd1.setCursor(0, 1);
        lcd1.print("Y AXIS: ");
        lcd1.print(result);
        lcd1.print("   ");

      }
    }
    //set the old data with the new to check if its been updated
    y_oData = yData;
    y_index = 0; //reset the index
    delay(5); //a delay?
    yData = 0; //reset the data
  }
}



