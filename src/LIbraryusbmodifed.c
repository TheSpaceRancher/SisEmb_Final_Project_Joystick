#include <18F4550.h>

#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN, XT
#use delay(clock=48000000, crystal=20000000,USB_FULL)

#include <pic18_usb.h>
#include <joy_control.h>
#include <usb.c>

#define JOY_AXIS_X 0
#define JOY_AXIS_Y 1

#define LED_RED PIN_B5
#define LED_GREEN PIN_B6
#define LED_BLUE PIN_B7

typedef struct buttons_state {
   int8 button_joystick;
   int8 button_RB;
   int8 button_LB;
   int8 button_Y;
   int8 button_B;
   int8 button_A;
   int8 button_X;
   int8 button_Up;
   int8 button_Left;
   int8 button_Right;
   int8 button_Down;
} buttons_state;

   buttons_state read_Buttons(){
   buttons_state reading;
   reading.button_joystick = input(PIN_B4)? 1 : 0;
   reading.button_RB = input(PIN_D4)? 1 : 0;
   reading.button_LB = input(PIN_D5)? 1 : 0;
   reading.button_Y = input(PIN_D0)? 1 : 0;
   reading.button_B = input(PIN_D1)? 1 : 0;
   reading.button_A = input(PIN_D2)? 1 : 0;
   reading.button_X = input(PIN_D3)? 1 : 0;
   reading.button_Up = input(PIN_B0)? 1 : 0;
   reading.button_Left = input(PIN_B1)? 1 : 0;
   reading.button_Right = input(PIN_B2)? 1 : 0;
   reading.button_Down = input(PIN_B3)? 1 : 0;
   return reading;
}


int8 adc_offset(int8 channel){
   int i;
   int32 sum = 0;
   for(i=0;i<8;i++){
      set_adc_channel(channel);
      delay_us(20);
      sum += read_adc();
      delay_ms(5);
   }
   return (int8)(((sum / 8) * 255) / 1023);
}

int8 read_joystick(int8 channel, int8 offset){
   set_adc_channel(channel);
   delay_us(20);
   
   int16 adc = read_adc();
   adc = ((adc*255) / 1023)- offset;
   adc = 0 - adc;
   return (int8)adc;
}

void main()
{
   usb_init_cs();

   
   int8 offsetX = 128;
   int8 offsetY = 128;

   buttons_state buttons;
   
   setup_adc_ports(AN0_TO_AN1_ANALOG);
   setup_adc(ADC_CLOCK_INTERNAL);

   set_tris_a(0b00000011);
   set_tris_b(0b00011111);
   output_b(0b00000000); 
   set_tris_d(0b00111111); 

   offsetX = adc_offset(0);
   offsetY = adc_offset(1);

   output_high(LED_RED);

   while(TRUE)
   {
      usb_task();

      if(usb_enumerated()){
         int8 btnmask = 0;
         int8 cruz = 0;

         buttons = read_Buttons();
            
            if(buttons.button_A) btnmask |= (1<<0);
            if(buttons.button_B) btnmask |= (1<<1);
            if(buttons.button_X) btnmask |= (1<<2);
            if(buttons.button_Y) btnmask |= (1<<3);
            if(buttons.button_LB) btnmask |= (1<<4);
            if(buttons.button_RB) btnmask |= (1<<5);

            if(buttons.button_Up) cruz |= (1<<0);
            if(buttons.button_Down) cruz |= (1<<1);
            if(buttons.button_Left) cruz |= (1<<2);
            if(buttons.button_Right) cruz |= (1<<3);

         int8 report[4];
         report[0] = btnmask;
         report[1] = read_joystick(JOY_AXIS_X, offsetX);
         report[2] = read_joystick(JOY_AXIS_Y, offsetY);
         report[3] = cruz;

         usb_put_packet(1, report, 4, USB_DTS_TOGGLE);
         }
         delay_ms(1);
   }
}
