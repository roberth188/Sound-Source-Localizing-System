//Sound Source Localization System-Three Dimensional
// 
// __________________B yellow
// 
// ______________________b
// 
// Note:  A red   a              E white  c           C green
// 
// ______________________d
// 
// ___________________D blue
// 
// Note: height time is positive when source-red/green is 
//       longer than source->white, or white->red/yellow/green/blue  
// Note: a = source->red; b = source->green; c = source->blue; d = source->white
// Note: inches and seconds favored initially
// Note: acos(-1min  1max);
// Note: red/green on x-axis, red positive.
//       before-yellow is Z, positive; underside is Y positive
//       later-yellow is Y positive; topside is Z positive

#include <math.h>                              //Sets-up needed libraries
#include <Servo.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(52, 50, 48, 46, 44, 42);   //Sets up LCD

int rxPin = 11;
int txPin = 10;

SoftwareSerial emicSerial =  SoftwareSerial(rxPin, txPin);

int red = 22;              //Sets up IO pins for the microphone input
int yellow = 24;
int green = 26;
int blue = 28;
int white = 30;

long errorontime;
int errorontag;

Servo servotheta;          //Moves Up/Down
Servo servophi;            //Azimuthal; moves horizontally, larger servo

float pointerdistancedifference = 2;           //servorhovalue - pointerdistancedifference = distancefrompointer

float distancefrompointer;       //Sets the scope of a variable

float xshift = 0;     //Tells of the offset for the pan/tilt system
float yshift = 11.4;
float zshift = 3.25;

float phitrim = 3;
float thetatrim = 8;

int servomovedelay = 800;       //Controls servo-related delay

int servoledpin = 40;       //Sets up the directional LED

int servoledonvar;

int speechinitwait;

int displaypin = 34;
int displaytag;

int redcount;       //Used in algorithm for amount to count each iteration
int yellowcount;
int greencount;
int bluecount;

int redtag;      //Used in algorithm to account for microphones
int yellowtag;
int greentag;
int bluetag;
int whitetag;

int whitetored;      //TDOA between microphones, in microseconds
int whitetoyellow;
int whitetogreen;
int whitetoblue;

int whitetoreda;
int whitetoyellowa;
int whitetogreena;
int whitetobluea;


int delaytimemicroseconds = 3;

float speedofsoundinchespersecond = 13397;        //13397.2441

float aininches = 8;

int limit = 150;           //75 * inches / delaytimemicroseconds
int limitnegative = -150;  //-limit

int echotime = 1000;     //A generic wait time, ie for echos

int printcount;         //The number of times that it has displayed an answer

float mysteryvalue = 11.8;     //A factor that makes the answer correct, due to delays, speed of sound differences, etc. 13.59 before, 11.7085 after

long servoledontime;

void setup()
{
  Serial.begin(9600);

  lcd.begin(20, 4);
  lcd.noAutoscroll();

  randomSeed(analogRead(3));            //For phrases in speaking

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  emicSerial.begin(9600);             //To connect the Speech synthesis module

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Initializing speech.");
  lcd.setCursor(0,2);
  lcd.print("   (Please wait)");

  emicSerial.print('\n');             // Send in case if up
  while (emicSerial.read() != ':' && speechinitwait <= 1000)     //Wait for it to turn on
  {
    emicSerial.print('\n');
    delay(10);
    speechinitwait ++;
  }
  delay(10);                          
  emicSerial.flush();                 // Flush the receive to clean communication

  lcd.clear();
  lcd.print("Generate a click to");
  lcd.setCursor(0,1);
  lcd.print("test. Please snap");
  lcd.setCursor(0,2);
  lcd.print("your fingers or use");
  lcd.setCursor(0,3);
  lcd.print("the clicking device.");


  pinMode(red, INPUT);
  pinMode(yellow, INPUT);
  pinMode(green, INPUT);
  pinMode(blue,INPUT);
  pinMode(white, INPUT);
  pinMode(servoledpin, OUTPUT);
  pinMode(displaypin, INPUT);

  digitalWrite(red, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(green, LOW);
  digitalWrite(blue, LOW);
  digitalWrite(white, LOW);
  digitalWrite(servoledpin, LOW);
  digitalWrite(displaypin, HIGH);

  delay(echotime);
}


void loop()  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  whitetored = 0;
  whitetogreen = 0;
  whitetoyellow = 0;
  whitetoblue = 0;

  whitetag = 0;               //Used in an error test

  if (digitalRead(red) == HIGH || digitalRead(yellow) == HIGH || digitalRead(green) == HIGH || digitalRead(blue) == HIGH || digitalRead(white) == HIGH)  //Begins algorithm when the first microphone hits
  {
    redcount = 0;        //Sets the value to count by as 0 for the algorithm- not hit yet
    yellowcount = 0;
    greencount = 0;
    bluecount = 0;

    redtag = 0;          //Tells if a microphone has registered yet; easy to follow
    yellowtag = 0;
    greentag = 0;
    bluetag = 0;
    whitetag = 0;

    while((redtag + yellowtag + greentag + bluetag + whitetag) < 5)    //Goes through an algorithm of adding or subtracting microseconds with delays before all microphones register and while within limits 
    {
      if (digitalRead(red) == HIGH && redtag == 0)                     //Red, ie, tests if registering and hasn't registered yet
      {
        redcount = redcount - delaytimemicroseconds;   //Makes it decrease- if before white, otherwise positive then stop
        redtag = 1;                                    //Notes that it has registered already so that redcount doesn't keep increasing
      }

      if (digitalRead(yellow) == HIGH && yellowtag == 0)    //Similar to red
      {
        yellowcount = yellowcount - delaytimemicroseconds;
        yellowtag = 1;
      }

      if (digitalRead(green) == HIGH && greentag == 0)    //Similar to red
      {
        greencount = greencount - delaytimemicroseconds;
        greentag = 1;
      }

      if (digitalRead(blue) == HIGH && bluetag == 0)    //Similar to red
      {
        bluecount = bluecount - delaytimemicroseconds;
        bluetag = 1;
      }

      if (digitalRead(white) == HIGH && whitetag == 0)     //Registering and hasn't registered
      {
        redcount = redcount + delaytimemicroseconds;          //When white hits before, it gives a positive TDOA; it stops the count (ie redcount = 0) if the color (ie red) hits before
        yellowcount = yellowcount + delaytimemicroseconds;     
        greencount = greencount + delaytimemicroseconds;
        bluecount = bluecount + delaytimemicroseconds;

        whitetag = 1;
      }

      whitetored = whitetored + redcount;                     //Counts by what is set
      whitetoyellow = whitetoyellow + yellowcount;
      whitetogreen = whitetogreen + greencount;
      whitetoblue = whitetoblue + bluecount;

      if(whitetored > limit || whitetored < limitnegative || whitetoyellow > limit || whitetoyellow < limitnegative || whitetogreen > limit || whitetogreen < limitnegative || whitetoblue > limit || whitetoblue < limitnegative)
      {
        goto error;            //If outside limits (ie if a microphone didn't hear the sound), goes to error
      }

      delayMicroseconds(delaytimemicroseconds);

    }                                                                 //End of while loop; goes back if any microphones haven't registered
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Formula Divider; easily visible
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (whitetored != 0 || whitetogreen != 0 || whitetoyellow != 0 || whitetoblue != 0)  //If this was an actual reading, this would make an infinite distance. Thus, a decent lock-out.
  {

    whitetoreda = whitetored;                 //Used to store these values for the ability to display them later.
    whitetoyellowa = whitetoyellow;
    whitetogreena = whitetogreen;
    whitetobluea = whitetoblue;
    
    if (whitetored + whitetogreen <= 0 || whitetoyellow + whitetoblue <= 0)              //Ie impossible values; located to save time
    {
      goto error;
    }
    
    float speedofsoundinchespermicrosecond = speedofsoundinchespersecond / 1000000;

    float redprepare = whitetored * mysteryvalue;                                                     //spooky; number of microseconds * program delay adjustment value
    float greenprepare = whitetogreen * mysteryvalue;
    float yellowprepare = whitetoyellow * mysteryvalue;                                                     
    float blueprepare = whitetoblue * mysteryvalue;

    float heightofred = speedofsoundinchespermicrosecond * redprepare;                               //actual number of microseconds * speed of sound as in per uS = heighth, or TDOA
    float heightofgreen = speedofsoundinchespermicrosecond * greenprepare;
    float heightofyellow = speedofsoundinchespermicrosecond * yellowprepare;
    float heightofblue = speedofsoundinchespermicrosecond * blueprepare;

    float distancerg = ((2 * (aininches * aininches)) - (heightofred * heightofred) - (heightofgreen * heightofgreen)) / abs((2 * heightofred) + (2 * heightofgreen)); //The formula for distance
    float distanceyb = ((2 * (aininches * aininches)) - (heightofyellow * heightofyellow) - (heightofblue * heightofblue)) / abs((2 * heightofyellow) + (2 * heightofblue));

    float distance = (abs(distancerg) + abs(distanceyb)) / 2;                                                                   /////////////////////////////Distance average

    float redininches = heightofred + distance;                        //Actual distance from each microphone to the source
    float blueininches = heightofblue + distance;

    float asquared = aininches * aininches;
    float bsquared = distance * distance;

    float redsquared = redininches * redininches;
    float bluesquared = blueininches * blueininches;

    float cosineanglered = (asquared + bsquared - redsquared) / (2 * (aininches * distance));                    //Different variable names, same formula
    float cosineangleblue = (asquared + bsquared - bluesquared) / (2 * (aininches * distance));

    float angleredrad = acos(cosineanglered);
    float anglebluerad = acos(cosineangleblue);

    //I now have the distance and angle values.

    float xcoordinate = distance * cosineanglered;                                    //Switched from z
    float ycoordinate = distance * cosineangleblue;                                 //Switched from x, then adapted from z
    
    if (abs(xcoordinate) <= 3 && abs(ycoordinate) <= 3)
    {
      xcoordinate = xcoordinate * 2.54;
      ycoordinate = ycoordinate * 2.54;
    }

    float zcoordinatefromanglered = sqrt(abs(sq(distance * sin(angleredrad)) - sq(ycoordinate)));   //Switched from y, then adapted from x and y (these)
    float zcoordinatefromangleblue = sqrt(abs(sq(distance * sin(anglebluerad)) - sq(xcoordinate)));

    float zcoordinate = (zcoordinatefromanglered + zcoordinatefromangleblue) / 2;                  //Averaged to make a more accurate value

    if (!(zcoordinate >= 0 && zcoordinate <= 1000))
    {
      zcoordinate = zcoordinatefromanglered;
    }

    if (!(zcoordinate >= 0 && zcoordinate <= 1000))
    {
      zcoordinate = zcoordinatefromangleblue;
    }

    if (!(zcoordinate >= 0 && zcoordinate <= 1000))
    {
      zcoordinate = 1.4;                 //Usable values are needed. A "nan" error is seen only at this height.
    }

    float distancereal = sqrt(abs(sq(xcoordinate) + sq(ycoordinate) + sq(zcoordinate)));

    float servorhovalue = sqrt(abs(sq(xcoordinate - xshift) + sq(ycoordinate - yshift) + sq(zcoordinate - zshift)));
    float servothetavalue = 57.2957795 * asin((zcoordinate - zshift)/servorhovalue);                              
    float servophivalue = 57.2957795 * acos(((0 - ycoordinate) + yshift)/sqrt(abs(sq(xcoordinate - xshift) + sq((0 - ycoordinate) + yshift))));

    if ((xcoordinate - xshift) < 0)             //Needed to account for the output limits of the trigonometric functions
    {
      servophivalue = (180 - servophivalue) - 15;
      servothetavalue = (180 - servothetavalue) + 15;
    }

    //Now, the values will be displayed


    printcount ++;

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("#");
    lcd.print(printcount % 1000);
    lcd.setCursor(4,0); 
    lcd.print(" Dist.= ");
    lcd.print(distancereal * 2.54);
    lcd.print("cm");

    lcd.setCursor(0,1);
    lcd.print("Coords.: X= ");
    lcd.print(xcoordinate * 2.54);     //Converted to centimeters
    lcd.print("cm");

    lcd.setCursor(9,2);
    lcd.print("Y= ");
    lcd.print(ycoordinate * 2.54);     //Converted to centimeters
    lcd.print("cm");

    lcd.setCursor(9,3);
    lcd.print("Z= ");
    lcd.print(zcoordinate * 2.54);     //Converted to centimeters
    lcd.print("cm");

    displaytag = 1;

    errorontag = 0;

    digitalWrite(servoledpin, LOW);   //To be sure

    //Move servos

    servophivalue = servophivalue + phitrim;           //Important! Accounts for offsets between the servos and their base!
    servothetavalue = servothetavalue + thetatrim;

    if (servophivalue >= 179) //So the servos aren't stressed
    {
      servophivalue = 179;
    }

    if (servophivalue <= 1)
    {
      servophivalue = 1;
    }

    if (servothetavalue <= 1)
    {
      servothetavalue = 1;
    }

    if (servothetavalue >= 179)
    { 
      servothetavalue = 179;
    }

    int phraseselect = random(1,6);

    emicSerial.print('X');
    emicSerial.print('\n');
    emicSerial.print('\n');
    
    int speechwaitcount = 0;
    
    while (emicSerial.read() != ':' && speechwaitcount <= 30)
    {
      emicSerial.print('X');
      emicSerial.print('\n');
      emicSerial.print('\n');
      delay(10);
      
      speechwaitcount ++;
    }  //Waits until module is ready

    delay(10);                          
    emicSerial.flush();                 // Flush the receive to clean communication
    delay(10);

    emicSerial.print('X');
    emicSerial.print('\n');
    
    
    //Servos are moving while starts speaking

    distancefrompointer = servorhovalue - pointerdistancedifference;
    distancefrompointer = distancefrompointer  * 2.54;         //To centimeters

    emicSerial.print('X');
    emicSerial.print('\n');
    emicSerial.print("n4");
    emicSerial.print('\n');
    emicSerial.print("v7");
    emicSerial.print('\n');
    emicSerial.print("w200");
    emicSerial.print('\n');
    emicSerial.print("P1");
    emicSerial.print('\n');
    emicSerial.print("S");


    emicSerial.print("The sound source is ");

    emicSerial.print(distancefrompointer, 2);

    if (phraseselect==1)
    {
      emicSerial.print("__centimeters away from the lighdt's origin.");  //Mis-spelled to improve pronunciation
    }
    if (phraseselect==2)
    {
      emicSerial.print("__centimeters from the indicating lighdt's origin.");
    }
    if (phraseselect==3)
    {
      emicSerial.print("__centimeters from the pointing lighdt's origin.");
    }
    if (phraseselect==4)
    {
      emicSerial.print("__centimeters away from the lighdt's emitter.");
    }
    if (phraseselect==5)
    {
      emicSerial.print("__centimeters from the __blue lighdt's origin.");
    }

    emicSerial.print('\n');

    float phidelaytimey = (servophivalue - servophi.read()) / 180;
    float phidelaytime = abs(phidelaytimey) * servomovedelay;
    float thetadelaytimey = (servothetavalue - servotheta.read()) / 180;
    float thetadelaytime = abs(thetadelaytimey) * servomovedelay;

    float servomovedelayminimum = max(phidelaytime, thetadelaytime) + 400;

    servophi.attach(36);
    servotheta.attach(38);

    servotheta.write(servothetavalue);
    servophi.write(servophivalue);

    delay(servomovedelayminimum);

    servophi.detach();            //Cuts communication with the servos to avoid effects on the microphone array
    servotheta.detach();

    delay(10);

    //Turn on LED

    for(int countvar = 0; countvar <= 500; countvar ++)       //LED is turned on gradually
    {
      delayMicroseconds(countvar);
      digitalWrite(servoledpin, LOW);
      delayMicroseconds(500 - countvar);
      digitalWrite(servoledpin, HIGH);
    }

    digitalWrite(servoledpin, HIGH);

    servoledontime = 0;
    servoledonvar = 1;

  }

error:
  if (whitetored > limit || whitetored < limitnegative || 
    whitetoyellow > limit || whitetoyellow < limitnegative || 
    whitetogreen > limit || whitetogreen < limitnegative || 
    whitetoblue > limit || whitetoblue < limitnegative)
  {
    /*emicSerial.print('X');        //Ceases talking on the voice synthesis module.
     emicSerial.print('\n');
     */
    lcd.clear();
    printcount ++;
    lcd.print("#");
    lcd.print(printcount % 1000);
    lcd.print("    Error!");
    lcd.setCursor(0,1);
    lcd.print("Louder, please!");

    lcd.setCursor(4,2);
    
    if (redtag == 0)
    {
    lcd.print(", R");
    }
    if (yellowtag == 0)
    {
    lcd.print(", Y");
    }
    if (greentag == 0)
    {
    lcd.print(", G");
    }
    if (bluetag == 0)
    {
    lcd.print(", B");
    }
    if (whitetag == 0)
    {
    lcd.print(", W");
    }
    
    lcd.setCursor(0,2);
    lcd.print("Note:");

    lcd.setCursor(0,3);
    lcd.print("did not register.");

    errorontime = 0;
    errorontag = 1;

    delay(echotime);
  }

  if (whitetag == 1)
  {
    if (whitetored + whitetogreen <= 0 || whitetoyellow + whitetoblue <= 0)
    {
      /*emicSerial.print('X');        //Ceases talking on the voice synthesis module.
       emicSerial.print('\n');
       */
      lcd.clear();
      printcount ++;
      lcd.print("#");
      lcd.print(printcount % 1000);
      lcd.print("    Error!");
      lcd.setCursor(0,1);
      lcd.print("Waveform issue!");

      lcd.setCursor(0,2);
      if (whitetored + whitetogreen <= 0)
      {
      lcd.print("R-G error ");
      }
      if (whitetoyellow + whitetoblue <= 0)
      {
      lcd.print("Y-B error ");
      }

      lcd.setCursor(0,3);
      lcd.print("Try again, please!");

      delay(echotime);
    }
  }

  if (servoledontime >= 500000 && servoledonvar == 1)  //About 30 seconds, or seconds = minutes * value / 1 million;
  {
    digitalWrite(servoledpin, LOW);

    servoledonvar = 0;
  }

  if (errorontime >= 1000000 && errorontag == 1)  //About 30 seconds, or seconds = minutes * value / 1 million;
  {
    lcd.clear();
    lcd.print("Generate a click to");
    lcd.setCursor(0,1);
    lcd.print("test. Please snap");
    lcd.setCursor(0,2);
    lcd.print("your fingers or use");
    lcd.setCursor(0,3);
    lcd.print("the clicking device.");
    
    printcount = 0;

    errorontag = 0;
  }

  if (digitalRead(displaypin) == LOW && displaytag == 1)
  {

    lcd.setCursor(0,2);
    lcd.print(whitetoreda);
    lcd.setCursor(4,2);
    lcd.print(whitetoyellowa);
    lcd.setCursor(0,3);
    lcd.print(whitetogreena);
    lcd.setCursor(4,3);
    lcd.print(whitetobluea);


    displaytag = 0;
  }

  servoledontime ++;
  errorontime ++;

  delayMicroseconds(3);
}
