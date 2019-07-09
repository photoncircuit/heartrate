//define pin used
#define sec_flag flag.F0      //sec timer
#define start_flag flag.F1    //start flag
#define led PORTC.F0          //led & buzzer pin
#define buzzer PORTC.F1
#define enter PORTE.F0

char msec,sec,minutes,hour;       //declare variables
int pulse;
char count,flag;
char txt[10];
char group;
char temp;

//define timer intterupt
//interupt cycle within interupt flag expires
//1msec
 void interrupt(){	                        //clock that is needed to time the heart rate.

  if  (INTCON.T0IF){ 	                     //interupt control - timer 0 expired
      asm{clrwdt}	                        //clear the watchdog timer in assembly - prevents PIC from failing
      msec++;                             //msec+1
      if (msec >100){
         msec = 0;
         sec++;                          //if more than 100 then sec+1
         sec_flag=1;
           if (sec >=60){                //if sec>60 then min+1
             sec = 0;
             minutes++;
               if (minutes >=60){       //if min > 60 then hour+1
                  minutes = 0;
                  hour++;
                     if (hour >=24){
                        hour = 0;
                     }
               }
           }
       }
  }
//  TMR0=0x00;
  TMR0  = 0   ;
  INTCON = 0x20;       // Set T0IE, clear T0IF    - restore original values
}

//set function prototype
void beep();
void ok();
void fail();
void set_group();
void error();
void healthy();
void unhealthy();

char ip1;

//main routine
void main()
{
  ADCON1 =0b10001110;                   //setup analogue for heartbeat
  TRISA = 0b00000011;                   //setup porta as input
  TRISB = 0b00000000;                   //portb as output
  TRISC = 0b00000000;                   //portc & d as output
  TRISD = 0b00000000;
  TRISE = 0b00000111;
  PORTA = 0b00000000;                   //clears all ports
  PORTB = 0b00000000;
  PORTC = 0b00000000;
  PORTD = 0b00000000;
  PORTE = 0x00;
  OPTION_REG = 0b00000101;              //timer ratio for intterupt
//  T0CON = 0b11000101;
  LCD_Config(&PORTB,4,5,6,3,2,1,0);     //setup LCD port
  Lcd_Init(&PORTB);                     // Initialize LCD connected to PORTB
  Lcd_Cmd(LCD_CLEAR);                   // Clear display
  Lcd_Cmd(LCD_CURSOR_OFF);              // Cursor off
  Lcd_Cmd(LCD_SECOND_ROW);
  Lcd_Cmd(LCD_CLEAR);                   // Clear display
  lcd_out(1,1,"Heartbeat Meter");       //display intro at lcd
  delay_ms(2000);
  Lcd_Cmd(LCD_CLEAR);                   // Clear display
  Usart_init(9600);                     //setup serial Bluetooth communication with a baud rate of 9600 bps
  sec=0;                                //clears all registers
  count=0;
  pulse=0;
  start_flag=0;	                        //push button - when pressed, then will turn to 1 (on)
  group=0;
  Lcd_out(1,1,"Standby");               //display standby
//main loop
  do
  {

//   set_group();
   if(enter)                            //wait to press enter
   {
    Lcd_out(1,1,"Start  ");             //display start and start the counting flag
    start_flag=1;
   }

   if(start_flag)                       //if flag detected
   {
     if(sec_flag)                       //wait for 1 sec flag
     {
      sec_flag=0;
      count=count+1;                    //count + 1 after every 1 sec
     }

     if(count>=15)                      //if the count > 15 sec, execute the following
     {
      pulse=pulse*4;                    //multiply pulse (15 sec) by 4 to get beats per minute
      lcd_out(2,1,"BPM:");
      inttostr(pulse,txt);              //display pulse to the LCD
      lcd_chr_cp(txt[2]);               //display lcd arrays
      lcd_chr_cp(txt[3]);
      lcd_chr_cp(txt[4]);
      lcd_chr_cp(txt[5]);
      beep();                           //beep 2x
      beep();
      usart_write(pulse);               //send pulse to bluetooth
      temp = adc_read(0x01)*2/4;        //read temperature value by converting ADC
      Usart_write(temp);                //send temp to bluetooth
      lcd_out(1,1,"Temp:");             //display temperature value to lcd
      bytetostr(temp,txt);
      lcd_chr_cp(txt[0]);
      lcd_chr_cp(txt[1]);
      lcd_chr_cp(txt[2]);
      count=0;
 //     pulse=0;
      start_flag=0;
      if(pulse>=95)
      lcd_out(1,10,"High");
      else if(pulse>=60)
      lcd_out(1,10,"Med ");
      else
      lcd_out(1,10,"Low ");
      delay_ms(3000);                   //delay 3sec


      Lcd_Cmd(LCD_CLEAR);               // Clear display
      Lcd_out(1,1,"Standby");           //display standby again
      pulse=0;
     }

     ip1 = adc_read(0)/4;               //read as analogue convert to digital
     if(ip1>=128)                       //if input > 128 == 2.5Volts as  a high peak
     {
      led=1;                            //led on
      buzzer=1;                         //buzzer
      pulse=pulse+1;                    //pulse  + 1
      while(1)                          //wait for low peak
      {
       if(sec_flag)                     //wait for 1sec flag
       {
        sec_flag=0;
        count=count+1;                  //keep counting sec
       }
       if(count>15)                     //if counter 15 sec then timeout
       break;
       ip1=adc_read(0)/4;               //convert adc
       if(ip1<=45)                      //if ip <45 == 0.7 volts
       {
        led=0;                          //off led
        buzzer=0;                       //off led
        break;                          //break from loop
       }
      }
     }


    }

  }while(1);
}

//****************************************

void beep(void)             //beeps the buzzer
{
 buzzer=1;
 delay_ms(100);
 buzzer=0;
 delay_ms(100);
}

