#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include <Keypad.h>
#include <EEPROM.h>

#define buzzer 9
#define motor_fr 10
#define motor_bk 11
#define echoPin 12
#define ir_sensor 13
#define lcd_col 20
#define lcd_row 4

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

void open_door();
int prompt_action_choice();
void change_password();
void raise_alarm(int duration);
String prompt_password();

void setup() {
  // put your setup code here, to run once:
  pinMode(motor_fr, OUTPUT); 
  pinMode(motor_bk, OUTPUT); 
  pinMode(buzzer, OUTPUT); // Set buzzer as an output
  
//  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
//  pinMode(echoPin, INPUT);

  // Initialize the IR Sensor
  pinMode(ir_sensor,INPUT);
  
  lcd.begin(lcd_col,lcd_row);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome Home .... ");
  lcd.setCursor(0,1);
  lcd.print("MR. " + user_name);
  delay(5000);
  lcd.clear();

  // Load System Password from memory
  // Password storage in memory
  // [Pass_length] [NUMNUM] [NUMNUM] [NUMNUM] ....
  String system_password;
  int pass_length = EEPROM.read(0);
  
  // each 2 numbers are stored in 1 byte, hence length/2
  // Check if pass_length is more than 2 (mainly to prevent it loading on initial run, as there would be no password stored yet)
  if(pass_length < 2){
      for(int i=1; i<pass_length;i++){
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
  int ir_data = HIGH;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Come Closer ...");
  while(ir_data != LOW){ // IR sensor HIGH = No detection. LOW = Detection
    ir_data = digitalRead(ir_sensor);
  }
  // Prompt User for password. On 3 unsuccessful attempts, raise the alarm
  while(passcode_attempts < 3 && !pass_succ){
    user_passcode = prompt_password();
    if(def_passcode == user_passcode){
      pass_succ = true;
      lcd.clear();
      lcd.setCursor(0,0);
      break;
     }  
     else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Incorrect Password.");
      lcd.setCursor(0,1);
      lcd.print("Try again...");
      delay(2000);
      passcode_attempts++;
     }
  }
  if(passcode_attempts = 3 && !pass_succ){ // Incorrect Password entry after 3 attempts
    raise_alarm(5);
    delay(10000);
  }
  else{
    int choice = 0;
    // Open door
    choice = prompt_action_choice();
    while(choice <= 0){
       if(choice == 1){
        open_door();
        delay(10000); 
      }
      // Change Password
      else if(choice == 2){
        change_password();
        delay(2000);
      }
      else if(choice == -1)
      {
        choice = prompt_action_choice();
      } 
      // Incorrect input. Reprompts action selection
      else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Incorrect Input.");
        lcd.setCursor(0,1);
        lcd.print("Please Try again.");
        delay(3000);
        prompt_action_choice();
      }
    }
  }
  delay(5000);
}



// Open the Door using the motor (uses around 2s of motor work)
void open_door(){
  digitalWrite(motor_fr, HIGH);
  digitalWrite(motor_bk, LOW);
  delay(2000);
  digitalWrite(motor_fr, LOW);
  digitalWrite(motor_bk, HIGH);
}

// Prompt user to select Action from a menu by pressing the corresponding character on the keypad
int prompt_action_choice(){
  boolean activated = true
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Choose an action:");
  lcd.setCursor(0,1);
  lcd.print("F1: Open Door");
  lcd.setCursor(0,2);
  lcd.print("F2: Change Password");
  while(activated){
      keypressed = myKeypad.getKey();
    if(keypressed == 'A'){
      tone(buzzer, 2000, 50);
      activated = false;
      return 1;  
    } else if(keypressed =='B'){
      tone(buzzer, 2000, 50);
      activated = false;
      return 2;
    }
    else{
      return -1;
    }
  }
}

// Prompt user to enter the system stored password by pressing the keys on the keypad then pressing a special character (F1)
String prompt_password(){
  int k = 0;
  boolean activated = true;
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
      if (keypressed != NO_KEY){
        tone(buzzer, 2000, 50);
        if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
            keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
            keypressed == '8' || keypressed == '9' ) {
          password += keypressed;
          lcd.setCursor(k,2);
          lcd.print("*");
          k++;
        }
        else if(keypressed == 'A'){
          tone(buzzer, 2000, 50);
          activated = false;
          return password;  
        }
        else if(keypressed =='B'){
          tone(buzzer, 2000, 50);
          password = "";
          k=0;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Enter Password:");
          lcd.setCursor(0,1);
          lcd.print("Press F1 on done");
          lcd.setCursor(0,2);
        }
    }    
  }
}

// create a police siren effect
void raise_alarm(int duration){
  for(int i=0; i<duration; i++){
    tone(buzzer, 6000, 1000);
    tone(buzzer, 500, 1000);
  }
}

// Prompts user to enter new password, then confirm it. Stores password in EEPROM.
void change_password(){
  String first_passcode;
  String confirm_passcode;
  int len_passcode = 0;
  first_passcode = prompt_password();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Confirm new passcode");
  confirm_passcode = prompt_password();
  if(first_passcode == confirm_passcode){
    // Save passcode and return to menu.
    len_passcode = strlen(confirm_passcode.c_str());
    if(len_passcode%2 != 0){
      len_passcode++;
    }
    len_passcode = len_passcode;
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.write(0, len_passcode);
    char delim[] = "";
    char* ptr = strtok(confirm_passcode.c_str(), delim);
    while(ptr != NULL)
    {
      ptr = strtok(NULL, delim);
    }
    String str = "";
    for(int i=0;i<len_passcode;i++)
    {
      EEPROM.write(i+1,ptr[i]);
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Password changed. Returning ...");
    return;
  }
  else{ // Passwords don't match. Either go to main menu or retry
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Password mismatch.");
    lcd.setCursor(0,1);
    lcd.print("Select action.");
    lcd.setCursor(0,1);
    lcd.print("F1: Try again");
    lcd.setCursor(0,1);
    lcd.print("F2: Return to menu");
    boolean resolved = false;
    while(!resolved){
        keypressed = myKeypad.getKey();
        if(keypressed == 'A'){
        tone(buzzer, 2000, 50);
        resolved = true;
        change_password();  
      } else if(keypressed =='B'){
        tone(buzzer, 2000, 50);
        resolved = true;
        prompt_action_choice();
      }
    }
  }
}
