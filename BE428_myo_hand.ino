/*  BE428 - Surgery 3/Give Them the Finger - Fall 2016
 *  From the giant brain of Elizabeth Kenny.
 *  
 *  Operate Myoelectric prosthetic hand for fine motor control via arduino, 
 *  Myoware Myoelectric sensor (A0)
 *  NEMA 17 Stepper Motor w/ Easy Driver (Pins 2, 3, 6).
 *  
 *  Running serial output to monitor EMG interfacing, number of steps.
 *  
 */


//Declare pin functions (NEMA 17 Stepper Motor with EasyDriver shield)
#define stp 2
#define dir 3
#define EN  6

// constants for motion
#define MAXEMG 0.35          // emg associated with full fist ( max threshold)
#define FULLCURL 8000        // NUMBER OF STEPS NECESSARY TO FULLY CURL FINGERS INTO FIST

bool reverse = false;        // extend fingers when true

// initiate vars for reading EMGs
int sensorValue = 0;
float voltage = 0;
float maxEMG = 0;

//Declare variables for motor functions
int x;
int state;
int rotationSize;

// setup for motor
void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Begin motor control");
  Serial.println();

}

//Main loop
void loop() {

  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control

  // read the input on analog pin A0:
  sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);

  Serial.println(voltage);
  // Flex fingers
  if (voltage >= 0.25 && !reverse) {                      // voltage should be between 0 and 0.25 when arm is relaxed

    Serial.println("calculating rotation");
    rotationSize = calcRotation(voltage);                 // calc number of steps

    Serial.println("Detected!");
    StepForwardDefault(rotationSize);

    reverse = true;     // set reverse bool to true when servo has turned all the waay
  }
  // reverse ( extend )
  else if (voltage >= 0.25 && reverse) {
    rotationSize = rotationSize;
    Serial.println("Reverse detected!");
    ReverseStepDefault(rotationSize);                   // no polarity for EMG signal, so can only extend all the way

    reverse = false;                  // set reverse bool to false, finger is fully extended
  }
  resetEDPins();
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(EN, HIGH);
}

//Default microstep mode function ( for curling fingers)
void StepForwardDefault(int scale)
{
  Serial.println("Moving forward at default step mode.");
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  for (x = 1; x < scale; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

//Reverse default microstep mode function
void ReverseStepDefault(int motorMax)
{

  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  Serial.print("The number of steps is: ");
  Serial.println(motorMax);
  for (x = 1; x < motorMax; x++) //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  
}

int calcRotation(float emgVal)
{
  float scaleFactor;
  int numSteps = 0;
  bool isMax = false;
  float emg0 = emgVal;        

  while (!isMax) {                        // poll EMG while the max has not been reached
    sensorValue = analogRead(A0);   // next
    float emg1 = sensorValue * (5.0 / 1023.0);    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

    // at max of emg (emg has peaked, move fingers)
    if (emg1 < emg0)
    {
      if (emg0 > MAXEMG) {      // if above max, full grip

        scaleFactor = 1;
      }
      else {
        scaleFactor = (emg0) / MAXEMG;          // scaling factor to modify number of steps
      }
      if (scaleFactor == 0)                     // something needs to move if above the threshold
        scaleFactor = 0.1;
      else if (scaleFactor > 0)
        scaleFactor = scaleFactor;
      Serial.print("The scale factor is: ");
      Serial.println(scaleFactor);

      numSteps = round((int)(scaleFactor * FULLCURL));
      Serial.print("The number of steps is: ");
      Serial.println(numSteps);
      isMax = true;
      return numSteps;
    }
    else
      emg0 = emg1;
  }
}



