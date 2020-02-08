#include<SPI.h>
#include<SD.h>
#include<Stepper.h>
/* H1_3h0_9t0f1_4f0_3t3f3_5
 * H1_5h1_9t60f0_3f1_4t7f1_0f1_4
 */
File myFile;
const int numFingers = 8;
const int rightFing[numFingers] = {2, 3, 4, 5, A0, A1, A2, A3};
const int leftFing[numFingers] = {2, 3, 4, 5, A0, A1, A2, A3};
//const int limitSwitch[2] = {23,25}; //Limit switches coem in later
const int numKeys = 61; //#keys on the piano
const double stepsPerKey = 38.571;//stepsPerKey(limitSwitch[1], limitSwitch[2], numKeys);  // change this to fit the number of steps per revolution
Stepper myStepper1(stepsPerKey, 6, 7, 8, 9);
//Stepper myStepper2(stepsPerKey, 10, 11, 12, 13);
const int mostKeys = 5;
int index = 0;
int fin = 0;

typedef struct timeStamp{
  timeStamp* next;
  int stamp;
  int rightFingOn[mostKeys];
  int rightFingOff[mostKeys];
  int leftFingOn[mostKeys];
  int leftFingOff[mostKeys];
}timeStamp;

typedef struct handPos{
  timeStamp* firstStamp;
  int move1;
  int move2;
}handPos;

typedef struct handPos *nodeH;
typedef struct timeStamp *nodeT;
nodeH hand = NULL;
nodeT startTime = NULL;
nodeT visitTime = NULL;

String dataString = "";
long timeStart;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // put your setup code here, to run once:
  for(int i = 0; i < numFingers; i++){
    pinMode(rightFing[i], OUTPUT);
    pinMode(leftFing[i], OUTPUT);
    Serial.print("ran loop " + i);
  }
  //pinMode(limitSwitch[1],INPUT);
  //pinMode(limitSwitch[2],INPUT);

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  /*
     Importnant notes: 
     - a midi note on an 88 key piano has value from 21 to 108
     each value is for one key on the piano
     - This model currently only considers the on values for midi file
     off would also be needed to hold keys for different times
  */
  myFile = SD.open("test.txt", FILE_READ);

  //myStepper.setSpeed(300); dont know if this would be needed
  
  hand = (nodeH)malloc(sizeof(handPos));
  hand->firstStamp = NULL;
  timeStart = millis();
}

void loop() {
  if(fin == 0){
    readHand();
    dataString = "";
    index = 0;
    playHand(hand);
    deallocateHand();
    startTime = hand->firstStamp;
  }

}

void readHand() {
  
  int num = 0;
  int posNeg = 0;
  bool first = true;
  int onOff = 0;
  int fingOnOff = 0;
  int leftRight = 0;
  int rightOnIndex = 0;
  int rightOffIndex = 0;
  int leftOnIndex = 0;
  int leftOffIndex = 0;
  int i;
  nodeT temp;
  if(myFile.available()<= 0){
    fin = 1;
    return;
  }
  while(myFile.available() > 0 && myFile.peek() != '\n')
  {
    Serial.println((char)myFile.peek());
    dataString += (char)myFile.read();
  }
  myFile.read();
  dataString += '\0';
  
  Serial.println(dataString);
  while (dataString.charAt(index) != '\0') 
  {
    Serial.println(index);
    Serial.println("Haomiao");
    if (dataString.charAt(index) == 'H')
    {
      Serial.println("H");
      while(!isDigit(dataString[index])){
        (index)++;
      }
      while(isDigit(dataString[index])){
        num = (num*10) + (dataString[index] - '0');
        (index)++;
      }
      posNeg = num;
      num = 0;
      Serial.println(posNeg);
      if (posNeg == 0)
      {
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        hand->move1 = -num;
        num = 0;
        Serial.println(hand->move1);
      }
      else 
      {
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        hand->move1 = num;
        num = 0;
        Serial.println(hand->move1);
      }
      index--;
    }
    else if (dataString.charAt(index) == 'h') 
    {
      Serial.println("h");
      while(!isDigit(dataString[index])){
        (index)++;
      }
      while(isDigit(dataString[index])){
        num = (num*10) + (dataString[index] - '0');
        (index)++;
      }
      posNeg = num;
      Serial.println(posNeg);
      num = 0;
      if (posNeg == 0) 
      {
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        hand->move2 = -num;
        Serial.println(hand->move2);
        num = 0;
      }
      else 
      {
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        hand->move2 = num;
        Serial.println(hand->move2);
        num = 0;
      }
      index--;
    }
    else if (dataString.charAt(index) == 't') 
    {
      Serial.println("t");
      temp = createTimeNode();
      for(i = 0 ; i < 8; i++){
        temp->rightFingOn[i] = 0;
        temp->rightFingOff[i] = 0;
        temp->leftFingOn[i] = 0;
        temp->leftFingOff[i] = 0;
      }
      if (hand->firstStamp == NULL)
      {
        hand->firstStamp = temp;
        startTime = hand->firstStamp;
        visitTime = hand->firstStamp;
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        visitTime->stamp = num;
        Serial.println(visitTime->stamp);
        num = 0;
      }
      else
      {
        startTime = hand->firstStamp;
        visitTime->next = temp;
        visitTime = visitTime->next;
        while(!isDigit(dataString[index])){
          (index)++;
        }
        while(isDigit(dataString[index])){
          num = (num*10) + (dataString[index] - '0');
          (index)++;
        }
        visitTime->stamp = num;
        Serial.println(visitTime->stamp);
        num = 0;
      }
      index--;
    }
    else if (dataString.charAt(index) == 'f')
    {
      Serial.println("f");
      while(!isDigit(dataString[index])){
        (index)++;
      }
      while(isDigit(dataString[index])){
        num = (num*10) + (dataString[index] - '0');
        (index)++;
      }
      leftRight = num;
      Serial.println(leftRight);
      num = 0;
      index++;
      if(leftRight == 1)
      {
        if(dataString.charAt(index) == '1')
        {
          index++;
          while(!isDigit(dataString[index])){
            (index)++;
          }
          while(isDigit(dataString[index])){
            num = (num*10) + (dataString[index] - '0');
            (index)++;
          }
          visitTime->rightFingOn[rightOnIndex] = num;
          
          Serial.println(visitTime->rightFingOn[rightOnIndex]);
          num = 0;
          rightOnIndex++;
        }
        else
        {
          index++;
          while(!isDigit(dataString[index])){
            (index)++;
          }
          while(isDigit(dataString[index])){
            num = (num*10) + (dataString[index] - '0');
            (index)++;
          }
          visitTime->rightFingOff[rightOffIndex] = num;
          
          Serial.println(visitTime->rightFingOff[rightOffIndex]);
          num = 0;
          rightOffIndex++;
        }
      }
      else
      {
        if(dataString.charAt(index) == '0')
        {
          index++;
          while(!isDigit(dataString[index])){
            (index)++;
          }
          while(isDigit(dataString[index])){
            num = (num*10) + (dataString[index] - '0');
            (index)++;
          }
          visitTime->leftFingOn[leftOnIndex] = num;
          
          Serial.println(visitTime->leftFingOn[leftOnIndex]);
          num = 0;
          leftOnIndex++;
        }
        else
        {
          index++;
          while(!isDigit(dataString[index])){
            (index)++;
          }
          while(isDigit(dataString[index])){
            num = (num*10) + (dataString[index] - '0');
            (index)++;
          }
          visitTime->leftFingOff[leftOffIndex] = num;
          
          Serial.println(visitTime->leftFingOff[leftOffIndex]);
          num = 0;
          leftOffIndex++;
        }
        index--;
      }
      index--;
    }
    index++;
  }
    return;
}

int getNum(String dataString, int* index){
  int num = 0;
  while(!isDigit(dataString[*index])){
    (*index)++;
  }
  while(isDigit(dataString[*index])){
    num = (num*10) + (dataString[*index] - '0');
    (*index)++;
  }
  return num;
}

nodeT createTimeNode(){
  nodeT temp;
  temp=(nodeT)malloc(sizeof(timeStamp));
  temp->next=NULL;
  return temp;
}

void deallocateHand()
{
  nodeT temp = startTime;
  visitTime = startTime->next;
  hand->firstStamp = NULL;
  while(temp != NULL){
    Serial.println("GG");
    free(temp);
    temp = visitTime;
    visitTime = visitTime->next;
  }
  temp = NULL;
  return;
}

void playHand(nodeH hand)
{
  int onRight = 0;
  int offRight = 0;
  int onLeft = 0;
  int offLeft = 0;
  visitTime = hand->firstStamp;
  //myStepper1.step(hand->move1*stepsPerKey);
  //myStepper2.step(hand->move2*stepsPerKey);
  Serial.println("move");
  while(visitTime != NULL)
  {
    Serial.println("visitTime");
    while((millis() - timeStart) < (visitTime->stamp)) {}
    while (visitTime->rightFingOn[onRight] != 0 && onRight < 8) {
      digitalWrite(rightFing[visitTime->rightFingOn[onRight]], HIGH);
      Serial.print("fing1");
      Serial.println(visitTime->rightFingOn[onRight]);
      onRight++;
    }
    onRight = 0;
    while (visitTime->rightFingOff[offRight] != 0 && offRight < 8) {
      digitalWrite(rightFing[visitTime->rightFingOff[offRight]], LOW);
      Serial.print("fing2");
      Serial.println(visitTime->rightFingOff[offRight]);
      offRight++;
    }
    offRight = 0;
    while (visitTime->leftFingOn[onLeft] != 0 && onLeft < 8) {
      digitalWrite(leftFing[visitTime->leftFingOn[onLeft]], HIGH);
      Serial.print("fing3");
      Serial.println(visitTime->leftFingOn[onLeft]);
      onLeft++;
    }
    onLeft = 0;
    while (visitTime->leftFingOff[offLeft] != 0 && offLeft < 8) {
      digitalWrite(leftFing[visitTime->leftFingOff[offLeft]], LOW);
      Serial.print("fing4");
      Serial.println(visitTime->leftFingOff[offLeft]);
      offLeft++;
    }
    offLeft = 0;
    visitTime = visitTime->next;
  }
  return;
}
