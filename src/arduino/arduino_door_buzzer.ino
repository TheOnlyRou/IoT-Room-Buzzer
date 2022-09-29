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

// Keypad declarations
const byte ROWS = 4; //four rows
const byte COLS = 5; //four columns
char keypressed;
// Keypad Symbol definition and mapping
char keyMap[ROWS][COLS] = {
  {'1','2','3','A','-'},
  {'4','5','6','B','-'},
  {'7','8','9','C','-'},
  {'*','0','#','D','-'}
};
byte rowPins[ROWS] = {8,7,6,5}; //Row pinouts of the keypad
byte colPins[COLS] = {4,3,2,1}; //Column pinouts of the keypad

// Keypad Initialization
Keypad myKeypad = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS); 
// LCD Initialization
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 

void open_door();
int prompt_action_choice(boolean on_screen);
void change_password();
void raise_alarm(int duration);
String prompt_password();
void clear_eeprom();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
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
  delay(2000);
  lcd.clear();

    
}

void loop() {
  // put your main code here, to run repeatedly:
  // Load System Password from memory
  // Password storage in memory
  // [Pass_length] [NUMNUM] [NUMNUM] [NUMNUM] ....
  
  //clear_eeprom();
  String system_password;
  int pass_length = EEPROM.read(0);
  Serial.print("Loaded Password Length\n");
  Serial.print(pass_length);
  // each 2 numbers are stored in 1 byte, hence length/2
  // Check if pass_length is more than 2 (mainly to prevent it loading on initial run, as there would be no password stored yet)
  int int_passcode = 0;
  if(pass_length > 2){
      for(int i=1; i<=pass_length; i++){
        int num_digit = EEPROM.read(i);
        Serial.println(num_digit);
        int_passcode = int_passcode * 10 + num_digit;
        char* char_digit;
        itoa(int_passcode,char_digit,10);
        system_password += char_digit;
    }
    char rou [pass_length +1];
    itoa(int_passcode, rou, 10);
//    system_password = "" ;
//    system_password += rou;
    Serial.print("System Password\n");
    Serial.print(system_password);
    def_passcode = system_password;
    lcd.setCursor(0,0);
    lcd.print("User Password ...");
    lcd.setCursor(0,1);
    lcd.print("Loaded!");
    delay(2000);
  }
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
    Serial.print("LOG: SUCCESSFUL PASSWORD");
    if(def_passcode == user_passcode){
      pass_succ = true;
      lcd.clear();
      lcd.setCursor(0,0);
      Serial.print("LOG: SUCCESSFUL PASSWORD");
      break;
     }  
     else{
      Serial.print("LOG: UNSUCCESSFUL PASSWORD");
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
    choice = prompt_action_choice(false);
    Serial.print(choice);    

    if(choice > 0){      
       if(choice == 1){ // Open door
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
        choice = prompt_action_choice(true);
      } 
      // Incorrect input. Reprompts action selection
      else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Incorrect Input.");
        lcd.setCursor(0,1);
        lcd.print("Please Try again.");
        delay(3000);
        return prompt_action_choice(false);
      }
    }
  }
  delay(5000);
}



// Open the Door using the motor (uses around 2s of motor work)
void open_door(){
  lcd.clear();
  lcd.setCursor(0,0);
  Serial.print("LOG: OPENING DOOR\n");
  lcd.print("Opening Door...");
  digitalWrite(motor_fr, HIGH);
  digitalWrite(motor_bk, LOW);
  delay(2000);
  digitalWrite(motor_fr, LOW);
  digitalWrite(motor_bk, HIGH);
}

// Prompt user to select Action from a menu by pressing the corresponding character on the keypad
int prompt_action_choice(boolean on_screen){
  boolean activated = true;
  if(!on_screen){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Choose an action:");
    lcd.setCursor(0,1);
    lcd.print("Start: Open Door");
    lcd.setCursor(0,2);
    lcd.print("Stop:Change Password");
    Serial.print("LOG: OPTIONS PRINTED\n");
  }
  while(activated){
    keypressed = myKeypad.getKey();
    if(keypressed == '*'){
      Serial.print("LOG: KEY ENTERED F1: OPEN DOOR\n");
      tone(buzzer, 2000, 50);
      activated = false;
      return 1;  
    } else if(keypressed =='#'){
      Serial.print("LOG: KEY ENTERED F2: CHANGE PASSWORD\n");
      tone(buzzer, 2000, 50);
      activated = false;
      return 2;
    }
    else if(keypressed != NO_KEY){
      Serial.print("LOG: KEY ENTERED: UNALLOWED KEY\n");
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
  lcd.print("Press Start on done");
  
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
        else if(keypressed == '*'){
          tone(buzzer, 2000, 50);
          activated = false;
          return password;  
        }
        else if(keypressed =='#'){
          tone(buzzer, 2000, 50);
          password = "";
          k=0;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Enter Password:");
          lcd.setCursor(0,1);
          lcd.print("Press Start on done");
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
  Serial.print("LOG: CHANGING PASSWORD\n");
  String first_passcode;
  String confirm_passcode;
  int len_passcode = 0;
  first_passcode = prompt_password();
  Serial.print("LOG: FIRST PASSWORD ENTERED\n");
  Serial.print(first_passcode);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Confirm new passcode");
  delay(2000);
  confirm_passcode = prompt_password();
  Serial.print("\n LOG: SECOND PASSWORD ENTERED\n");
  Serial.print(confirm_passcode);
  
  if(first_passcode == confirm_passcode){
    // Save passcode and return to menu.
    Serial.print("LOG: PASSWORD MATCH. SAVING\n");
    len_passcode = strlen(confirm_passcode.c_str());
    
    
    clear_eeprom();
    EEPROM.write(0, len_passcode);
    char delim[] = "";
    char* ptr = strtok(confirm_passcode.c_str(), delim);
    Serial.println("ptr naw");
    Serial.println(ptr);
    //while(ptr != NULL)
    //{
      //ptr = strtok(NULL, delim);
    //}
   // String str = "";
    
    for(int i=0;i<len_passcode;i++)
    {
      int temp = (int)(ptr[i]) - 48;
      EEPROM.write(i+1,temp);
      Serial.println(temp);
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Password changed.");
    lcd.setCursor(0,1);
    lcd.print("Returning ...");
    return;
  }
  else{ // Passwords don't match. Either go to main menu or retry
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Password mismatch.");
    lcd.setCursor(0,1);
    lcd.print("Select action.");
    lcd.setCursor(0,2);
    lcd.print("Start: Try again");
    lcd.setCursor(0,3);
    lcd.print("Stop: Return to menu");
    boolean resolved = false;
    while(!resolved){
      Serial.print("LOG: PASSWORD MISMATCH. USER MUST SELECT ACTION\n");
        keypressed = myKeypad.getKey();
        if(keypressed == '*'){
        tone(buzzer, 2000, 50);
        resolved = true;
        change_password();  
      } else if(keypressed =='#'){
        tone(buzzer, 2000, 50);
        resolved = true;
        return prompt_action_choice(false);
      }
    }
  }
}

// Sets all EEPROM bits to 0, effectively clearing the microcontroller's data
void clear_eeprom(){
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }  
}
