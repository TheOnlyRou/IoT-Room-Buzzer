#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include <Keypad.h>
#include <EEPROM.h>

#define buzzer 9
#define trigPin 11
#define echoPin 12
#define motor_pin 10
#define ir_sensor 13

// User Data Declarations
String def_passcode = "0000";
String user_passcode;
String user_name = "User";

// State booleans for system state management
boolean alarm_state = false;
boolean correct_pass = true;
boolean pass_changed = false;

// Keypad declarations
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keypressed;
// Keypad Symbol definition and mapping
char keyMap[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {8,7,6,5}; //Row pinouts of the keypad
byte colPins[COLS] = {4,3,2,1}; //Column pinouts of the keypad

// Keypad Initialization
Keypad myKeypad = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS); 
// LCD Initialization
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 


void setup() {
  // put your setup code here, to run once:
  pinMode(motor_pin, OUTPUT); 
  pinMode(buzzer, OUTPUT); // Set buzzer as an output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);

  // Initialize the IR Sensor
  pinMode(ir_sensor,INPUT);
  
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome Home .... ");
  lcd.setCursor(0,1);
  lcd.print("MR" + user_name);
  delay(5000);
  lcd.clear();

  // Load System Password from memory
  // Password storage in memory
  // [Pass_length] [NUMNUM] [NUMNUM] [NUMNUM] ....
  String system_password;
  int pass_length = EEPROM.read(0);
  // each 2 numbers are stored in 1 byte, hence length/2
  // Check if pass_length is more than 4 (mainly to prevent it loading on initial run, as there would be no password stored yet)
  if(pass_length < 4){
      for(int i=1; i<pass_length/2;i++){
        String num_num = (char*)EEPROM.read(i);
        system_password += num_num;
    }
    def_passcode = system_password;
  }  
}

void loop() {
  // put your main code here, to run repeatedly:
  int passcode_attempts = 0;
  boolean pass_succ = false;
  boolean ir_data = digitalRead(ir_sensor);
  while(ir_data){ // IR sensor True = No detection. False = Detection
    lcd.print("Come Closer ...");
  }
  // Prompt User for password. On 3 unsuccessful attempts, raise the alarm
  while(passcode_attempts <= 3 && !pass_succ){
    user_passcode = prompt_password();
    if(def_passcode == user_password)
      pass_succ = true;  
  }
  if(passcode_attempts > 3 && !pass_succ){ // Incorrect Password entry after 3 attempts
    raise_alarm(5);
    delay(10000);
  }
  else{ // Correct Password entry
    // Open the Door using the motor (uses around 2s of motor work)
    digitalWrite(motor_pin, HIGH);
    delay(2000);
    digitalWrite(motor_pin, LOW);
  }
    
}

String prompt_password(){
  boolean activated = true;
  boolean typing = false;
  String password;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Password:");
  lcd.setCursor(0,1);
  lcd.print("Press F1 on done");
  
  // On Key press, clear screen, then enter an asterisk on every key press.
  // Stop recording password when user presses A (F1 on this keypad)
  while(activated){
      keypressed = myKeypad.getKey();
      tone(buzzer, 2000, 100);
      if(!typing){  
        lcd.clear();
        lcd.setCursor(0,0);
        typing = true;
      }
      if (keypressed != NO_KEY){
        if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
            keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
            keypressed == '8' || keypressed == '9' ) {
          password += keypressed;
          lcd.setCursor(k,1);
          lcd.print("*");
          k++;
        }
        else if(keypressed == 'A'){
          return password;  
        }
    }    
  }
}

void raise_alarm(int duration){
  // create a police siren effect
  for(int i=0; i<duration; i++){
    tone(buzzer, 3000, 100);
    tone(buzzer, 1000, 100);
  }
}
