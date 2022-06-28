#include <LiquidCrystal.h>
#define B1 2
#define B2 3
#define B3 8
#define B4 12
#define BUZZER 13
#define TEMP A0
#define INTERVAL_BUTTON 50
#define LONG_PRESS_TIME 3000

int temperature = 0;

LiquidCrystal lcd(11,10,4,5,6,9);

int ss=49,mm=59,hh=13,alHH=13,alMM=59, snoozeHH = 0, snoozeMM = 0;
bool setClock = false, setAlarm = false, alarmON = false;
bool mode24 = true, modeCelsius = false, snoozeAlarm = false;
bool setMinute = false, setHour = false;

unsigned long currentTime;

int lastState_B1 = LOW;
int lastState_B2 = LOW;

unsigned long pressedTime_B1,previousMillis_B1,PressDuration_B1;
unsigned long pressedTime_B2,previousMillis_B2,PressDuration_B2;

void setup()
{
    Serial.begin(9600);
    lcd.begin(16,2);
    DDRD = 0b00000000;
    DDRB = 0b00100000;

    lcd.setCursor(0,0);
    lcd.print("Mucahit BAYRAM");
    lcd.setCursor(0,1);
    lcd.print("CMP3010");
    delay(3000);
    lcd.clear();



    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 15624;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);

    sei();

    lcd.setCursor(0,0);

    attachInterrupt(digitalPinToInterrupt(B1),b1INTERRUPT,RISING);
    attachInterrupt(digitalPinToInterrupt(B2),b2INTERRUPT,RISING);

}
void loop(){

    if(ss>=60)
    {
        ss-=60;
        mm++;
        if(mm==60)
        {
            mm=0;
            hh++;

            if(hh==24)
            {
                hh=0;
            }

        }

    }

    if(hh == alHH && mm == alMM && alarmON){
        PORTB = PORTB | 0b00100000;
    }
    else{
        PORTB = PORTB & 0b11011111;
    }
    if(snoozeAlarm && hh == snoozeHH && mm == snoozeMM){
        if(digitalRead(B2) == HIGH){
            snoozeAlarm = false;
        }
        alarmON = true;
        PORTB = PORTB | 0b00100000;
    }


    blink();
    temperature_C_or_F();
    snoozeAlarmFunction();
    changeMode24();
    changeAlarm();
    temperature = map(((analogRead(A0) - 20) * 3.04), 0, 1023, -40, 125);
    currentTime = millis();
    readB1State();
    readB2State();
    increase();
    printOnScreen();
}

void printTime(int x)
{
    if(x<10)
    {
        lcd.print("0");
        lcd.print(x);
    }
    else
    {
        lcd.print(x);
    }

}

void printOnScreen()
{

    lcd.setCursor(0,0);
    if(mode24 || hh <= 12){
        printTime(hh);lcd.print(":");
    }
    else{
        printTime((hh-12));lcd.print(":");
    }


    printTime(mm);lcd.print(":");
    printTime(ss);lcd.print(" ");
    lcd.setCursor(9,0);
    if(!mode24){
        if(hh>11)
        {
            lcd.print("PM");
        }
        else
        {
            lcd.print("AM");
        }
    }
    else{
        lcd.print("  ");
    }


    lcd.setCursor(0,1);
    if(modeCelsius){
        lcd.print(temperature);lcd.print(" C ");
    }
    else{
        lcd.print((temperature * 9) / 5 + 32);lcd.print(" F ");
    }


    lcd.setCursor(13,0);
    if(alarmON){
        lcd.print("ON ");
    }
    else{
        lcd.print("OFF");
    }

    lcd.setCursor(11,1);
    printTime(alHH);lcd.print(":");
    printTime(alMM);


}


void b1INTERRUPT(){
    if(setClock){
        if(setMinute){
            setMinute = !setMinute;
            setHour = true;
        }
        else{
            setHour = false;
            setClock = false;
        }
    }
}
void changeMode24(){
    if(digitalRead(B1) == HIGH && !setClock){
        mode24 = !mode24;
    }
}
void b2INTERRUPT(){
    if(setAlarm){
        if(setMinute){
            setMinute = !setMinute;
            setHour = true;
        }
        else{
            setHour = false;
            setAlarm = false;
        }
    }

}
void changeAlarm(){
    if(digitalRead(B2) == HIGH && !setAlarm){
        alarmON = !alarmON;
    }
}
void blink(){
    if (setClock && setHour){delay(100);lcd.setCursor(0,0);  lcd.print("  ");}
    if (setClock && setMinute){delay(100);lcd.setCursor(3,0);  lcd.print("  ");}
    if (setAlarm && setHour){delay(100);lcd.setCursor(11,1);  lcd.print("  ");}
    if (setAlarm && setMinute){delay(100);lcd.setCursor(14,1);  lcd.print("  ");}

    delay(100);
}
void temperature_C_or_F(){
    if(!setClock && !setAlarm && digitalRead(B3) == HIGH){
        modeCelsius = !modeCelsius;
    }
}
void increase(){
    if (setClock && setHour && digitalRead(B3) == HIGH){
        hh++;
        if(hh == 24){
            hh=0;
        }
    }
    if (setClock && setMinute && digitalRead(B3) == HIGH){
        mm++;
        if(mm == 60){
            mm = 0;
        }
    }
    if (setAlarm && setHour && digitalRead(B3) == HIGH){
        alHH++;
        if(alHH == 24){
            alHH=0;
        }
    }
    if (setAlarm && setMinute && digitalRead(B3) == HIGH){
        alMM++;
        if(alMM == 60){
            alMM = 0;
        }
    }
}

void readB1State() {

    if(currentTime - previousMillis_B1 > INTERVAL_BUTTON) {

        int buttonState = digitalRead(B1);

        if (buttonState == HIGH && lastState_B1 == LOW && !setClock) {
            pressedTime_B1 = currentTime;
            lastState_B1 = HIGH;
        }

        PressDuration_B1 = currentTime - pressedTime_B1;

        if (buttonState == HIGH && !setClock && PressDuration_B1 >= LONG_PRESS_TIME) {
            setClock = true;
            setMinute = true;
            Serial.println(PressDuration_B1);
        }

        if (buttonState == LOW && lastState_B1 == HIGH) {
            lastState_B1 = LOW;

        }

        previousMillis_B1 = currentTime;

    }

}
void readB2State() {

    if(currentTime - previousMillis_B2 > INTERVAL_BUTTON) {

        int buttonState = digitalRead(B2);

        if (buttonState == HIGH && lastState_B2 == LOW && !setAlarm) {
            pressedTime_B2 = currentTime;
            lastState_B2 = HIGH;
        }

        PressDuration_B2 = currentTime - pressedTime_B2;

        if (buttonState == HIGH && !setAlarm && PressDuration_B2 >= LONG_PRESS_TIME) {
            setAlarm = true;
            setMinute = true;
            Serial.println(PressDuration_B2);
        }

        if (buttonState == LOW && lastState_B2 == HIGH) {
            lastState_B2 = LOW;
        }
        previousMillis_B2 = currentTime;
    }

}
void snoozeAlarmFunction(){
    if (digitalRead(B4) == HIGH && hh == alHH && mm == alMM && alarmON){
        snoozeAlarm = true;
        snoozeMM = alMM + 5;
        snoozeHH = alHH;
        if(snoozeMM >= 60){
            snoozeMM -= 60;
            snoozeHH++;
            if(snoozeMM > 24){
                snoozeHH -= 24;
            }
        }
        alarmON = false;
    }

}
ISR(TIMER1_COMPA_vect)
        {
                ss++;
        }
