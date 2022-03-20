#define OUT_A_PIN  2
#define OUT_B_PIN  3
#define BUTTON_PIN 9

#define MAX_COUNT 100
#define MIN_COUNT 0

volatile int out_A = 0;
volatile int out_B = 0;

int count = 0;
int aState_cur;
int aState_last;
 
void setup() {
  Serial.begin(9600);

  // Rotary Encoder Pins
  pinMode(OUT_A_PIN, INPUT);
  //attachInterrupt();
  pinMode(OUT_B_PIN, INPUT);
  //attachInterrupt();

  // Push button
  pinMode(BUTTON_PIN, INPUT);
  // Pull high
  digitalWrite(BUTTON_PIN, HIGH);

  aState_last = digitalRead(OUT_A_PIN);
}

void loop() {
  // Encoder Logic
  aState_cur = digitalRead(OUT_A_PIN);
  if(aState_cur != aState_last){
    if(digitalRead(OUT_B_PIN) != aState_cur){
      if(count > MIN_COUNT){
        count--;
      }
    }
    else{
      if(count < MAX_COUNT){
        count++;
      }
    }

    Serial.print("Position: ");
    Serial.println(count);
  }
  aState_last = aState_cur;

  // Button Logic
  if(digitalRead(BUTTON_PIN) == 0){
    Serial.println("BUTTON PRESSED");
    while(digitalRead(BUTTON_PIN) == 0);
  }
}
