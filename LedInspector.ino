/*
LED Inspector
This program is used to enable Arduino to gradually increase the voltage accross the test LED and at the same time, the voltage accross the serial resistor is measured.
If the current flowing through the LED (as calculated by dividing the voltage accross the serial resistor by its ohmage) reaches the rated value (20 mAmps), 
then the operation point is reached. The resulting operational voltage is displayed on the LCD along with the suitable serial resistor in case of 5, 9 and 12 volts of 
supply voltateg
*/

#include <Wire.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <SSD1331.h>
#include <S65L2F50.h>
#include <S65LPH88.h>
#include <S65LS020.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayI2C.h>
typedef uint8_t prog_uint8_t;

MI0283QT2 lcd;  //MI0283QT2 Adapter v1
long measureInterval = 50;// Time allowed to measure voltage in millisecond.
long serialResistor = 60; // Serial resistor value in Ohms
double ratedCurrent = 0.02;


void setup()
{
  uint8_t clear_bg=0x00; //0x80 = dont clear background for fonts (only for DisplayXXX)
  //init display
  lcd.begin();
  //clear screen
  lcd.fillScreen(RGB(255,0,0));
  
  //prepare to print
  lcd.setTextSize(1|clear_bg);
  lcd.setTextColor(RGB(200,0,0), RGB(255,255,255));
  lcd.setCursor(1, 1);  

// setting baud rate for the serial port
  Serial.begin(9600); 
}


void loop()
{
double opVolt = inspectOpVolt(A1, 3,4.1);
displayResultsOnLed(opVolt);
//displayResultsOnLed(2.2);
//displayResultsOnPC(2.2);
displayResultsOnPC(opVolt);
delay (10000);
}

/*
This method reads the voltage of a given analog input pin.
Since the PWM produces non-smooth wave form, there is a need to get a specific value from this fluctuating waveform. The method reads the voltage within 
a predefined duartion and gets the maximum value.

parameters:
pintNumber: The pin ID of the analog input pin
baseVolt: maximum voltage acrros the LED and series resistor
pwmVal: the current value of the PWM (from 0 to 255)
*/
double readVolt(int pinNumber, double baseVolt,double pwmVal)
{
long startTime = millis();
double measuredVoltage = 0;
double instaVoltage = 0;
while (millis()- startTime <= measureInterval)
 {
  instaVoltage = analogRead(pinNumber);
  if (instaVoltage > measuredVoltage) measuredVoltage = instaVoltage;
 }
 writeLog("Before return readVolt relative:", (measuredVoltage));
 measuredVoltage = (measuredVoltage/1024)*baseVolt*(pwmVal/256);
 writeLog("Before return readVolt absolute:", (measuredVoltage));
 return (measuredVoltage);
}

/*
 * This method displays the test result on the LCD screen. 
 * input
 * *****
 * opVolt: LED operating voltage as calculated by Arduino or -1 in case of error
 * The method displays the operating voltage along with the recommended serial resistor to be connected to the tested LED in case of 5, 9 and 12 volts of input voltage.
 * 
 */
void displayResultsOnLed(double OpVolt)
{

  lcd.fillScreen(RGB(255,255,255));
  lcd.setCursor(1, 1);

  if (OpVolt<0)
  {
    lcd.println("An error was encountered");
  }
  else
  {
  lcd.println("Test Results:");
  lcd.print("Operating voltage: ");
  lcd.print(OpVolt);
  lcd.println(" Volts.");
  
  lcd.print("Serial Resistor @ 5 Volts: ");
  lcd.print((5- OpVolt)/0.02);
  lcd.println(" Ohms.");

  lcd.print("Serial Resistor @ 9 Volts: ");
  lcd.print((9- OpVolt)/0.02);
  lcd.println(" Ohms.");

  lcd.print("Serial Resistor @ 12 Volts: ");
  lcd.print((12- OpVolt)/0.02);
  lcd.println(" Ohms.");
  }
  delay (2000);
}

/*
 * This method prints the test results to the serial port thus making it possible to view the results on PC screen. 
 * input
 * *****
 * opVolt: LED operating voltage as calculated by Arduino or -1 in case of error
 * The method displays the operating voltage along with the recommended serial resistor to be connected to the tested LED in case of 5, 9 and 12 volts of input voltage.
 * 
 */
void displayResultsOnPC(double OpVolt)
{
  if (OpVolt<0)
  {
    Serial.println("An error was encountered");
  }
  else
  {
  Serial.println("Test Results:");
  Serial.print("Operating voltage: ");
  Serial.print(OpVolt);
  Serial.println(" Volts.");
  
  Serial.print("Serial Resistor @ 5 Volts: ");
  Serial.print((5- OpVolt)/0.02);
  Serial.println(" Ohms.");

  Serial.print("Serial Resistor @ 9 Volts: ");
  Serial.print((9- OpVolt)/0.02);
  Serial.println(" Ohms.");

  Serial.print("Serial Resistor @ 12 Volts: ");
  Serial.print((12- OpVolt)/0.02);
  Serial.println(" Ohms.");
  }
  delay (2000);
}
/*
 * This method displays the test progress (in %) both on LCD screen as well as serial port (to be displayed on PC screen) 
 * input
 * *****
 * percent: The percentage of LED inspection completion (0 to 100%)
 * 
 */
void displayTestProgress(int percent)
{
  lcd.fillScreen(RGB(255,255,255));
  lcd.setCursor(1, 1);

  lcd.print("Test in progress: ");
  lcd.print(percent);
  lcd.print(" %");

  Serial.print("Test in progress: ");
  Serial.print(percent);
  Serial.println(" %");
  delay (150);
}

/*
 * This is the main method for testing the LED. it gradually increases the voltage accross the LED + resistor and at the same time measures the voltage accross the serial resistor 
 * Since the resistor has a predefined value, the current accross it can be calculated.
 * When the current value reaches or exceeds 20 m Amps, then the operating voltage of the LED is reached and it will be equal to the voltage accross the LED resistor minus
 * the voltage drop accross the resistor
 * input
 * *****
 * serResPin: The arduino analog input pin connected to the serial resistor
 * outputPin: The Arduino PWM analog output pin connecte dto the LED-resistor
 * baseVolt: The maximum voltage accross the LED-Resistor in volts
 * 
 * output
 * ***** 
 * The calculated operating voltage in volts.
 */
double inspectOpVolt(int serResPin, int outputPin,double baseVolt)
{
double instaVolt;
double opVolt = 0;
double i = 0;  
while (i< 256)
{
analogWrite (outputPin,i);
displayTestProgress(((i*100)/256));
instaVolt = readVolt(serResPin,4.94,i);

writeLog ("PWM value: ",i);
writeLog ("Instantaneous resistor voltage: ",instaVolt);
if ((instaVolt/serialResistor) >= ratedCurrent)
{
  opVolt = ((baseVolt)*(i/256) - instaVolt);
  writeLog ("Operational voltage: ",opVolt);
  
  return opVolt;
}
i=i+10;
}
return -1;
}

/*
 * The following methods writes log entries in the form of text and numeric value
 * The methods are overloaded to support several numeric data types
 */
void writeLog(String message, String value)
{
Serial.print(message);
Serial.println(value);
}

void writeLog(String message, int value)
{
Serial.print(message);
Serial.println(value);
}

void writeLog(String message, double value)
{

Serial.print(message);
Serial.println(value);
}
