#include<reg51.h>

sbit RS = P0^0;        //To enable the LCD screen
sbit EN = P0^1;

sbit dht11 = P0^5;
sbit fan = P0^4;
sbit soil = P0^2;   
sbit motor = P0^3;    //Initialisation of all sensors
sbit ldr = P0^6; 
sbit led	=	P3^7;
sbit trig = P3^4;


void cmd_to_lcd(unsigned char cmd);
void data_to_lcd(unsigned char Data);   //used for LCD
void MSdelay(unsigned int time);
void initial_delay();

void temp_hum();
void soil_moist();
void light_inten();
void ultrasonic();
void display(unsigned int target_range);
void bluetooth_sending();
char bt_data[8];

void ISR_Serial() interrupt 4{
	char str;
	str=SBUF;
	
	if(str=='1'){
		fan=0;
	}
	if(str=='2'){
		fan=1;
	}
		
	if(str=='3'){
		motor=0;
	}
	
	if(str=='4'){
		motor=1;
	}
	if(str=='5'){
		led=0;
	}
	if(str=='6'){
		led=1;
	}
	RI=0;
}

void main(){
	
	IE=0X90;      //interuppt 
	TMOD=0X20;
	TH1=0XFD;
	SCON=0X50;
	TR1=1;
	
	cmd_to_lcd(0x38);            //to initialize the screen
	MSdelay(100);
	
	cmd_to_lcd(0x0F);            //Display on Cursor Blinking
	MSdelay(100);
	
	while(1){
		cmd_to_lcd(0X80);
		
		data_to_lcd('T');
		data_to_lcd('=');
		temp_hum();
		
		cmd_to_lcd(0XC0);

		data_to_lcd('S');
		data_to_lcd('=');
	  soil_moist();
		
		data_to_lcd('L');
		data_to_lcd('=');
	  light_inten();
		
		data_to_lcd('W');
		data_to_lcd('T');
		data_to_lcd('R');
		data_to_lcd('=');
		ultrasonic();
		
		bluetooth_sending();
	
		MSdelay(100);
	}
	
}

void bluetooth_sending(){
	int i;
	TMOD=0X20;
	TH1=0XFD;
	SCON=0X50;
	TR1=1;
	for(i=0;i<8;i++)
	{
    SBUF=bt_data[i];
    while(TI==0);
    TI=0;
		MSdelay(50);
	}
}

void temp_hum(){
	unsigned int bytes,bits;
	unsigned char rec_byte,data_array[5],bit_time,disp[3];
	MSdelay(50);
	dht11=1;
	TMOD=0x21;
	dht11=0;
	initial_delay();
	dht11=1;
	TMOD=0x22;
	TL0=0;
	TH0=0;
		while(dht11);		//initially high
		while(!dht11);		//start low
		while(dht11);		//start high
		for(bytes=0;bytes<5;bytes++){
			for(bits=0;bits<8;bits++){
				while(!dht11);
				TR0=1;
				while(dht11);
				TR0=0;
				bit_time=TL0;
				TL0=0;
				rec_byte=rec_byte<<1;
				if(bit_time>50)
					rec_byte=rec_byte|1;
			}
			data_array[bytes]=rec_byte;
		}
		MSdelay(50);
		disp[0]=data_array[2]%10;
		disp[1]=(data_array[2]/10)%10;
		disp[2]=data_array[2]/100;
		for(bytes=3;bytes>0;bytes--){
			data_to_lcd(disp[bytes-1]+0x30);
			MSdelay(50);
		}

		data_to_lcd('*');
		data_to_lcd('C');
		data_to_lcd(' ');
		data_to_lcd(' ');
		
		bt_data[0]=disp[1]+0x30;
		bt_data[1]=disp[0]+0x30;
		
		data_to_lcd('H');
		data_to_lcd('=');
		disp[0]=data_array[0]%10;
		disp[1]=(data_array[0]/10)%10;
		disp[2]=data_array[0]/100;
		for(bytes=3;bytes>0;bytes--){
			data_to_lcd(disp[bytes-1]+0x30);
			MSdelay(50);
		}
		data_to_lcd('%');
		
		bt_data[2]=disp[1]+0x30;
		bt_data[3]=disp[0]+0x30;
		
}

void soil_moist(){
	char ch;
	if(soil){
		ch='Y';
		data_to_lcd(ch);
	}
	else{
		ch='N';
		data_to_lcd(ch);
	}
	data_to_lcd('|');
	bt_data[4]=ch;
}

void light_inten(){                                             
	char ch;
	if(ldr){
		ch='Y';
		data_to_lcd(ch);
	}
	else{
		ch='N';
		data_to_lcd(ch);
	}
	data_to_lcd('|');
	bt_data[5]=ch;
}

void ultrasonic(){
	
	unsigned int target_range=0,timer_val;
	P3|=0x04;		//P3.2 as INPUT FOR ECHO
		TMOD=0x21;
		TH0=0xFF;
		TL0=0xF5;
		trig=1;
		TR0=1;
		while(!TF0);
		TR0=0;
		TF0=0;
		TMOD=0x29;		//timer0 in 16 bit mode with gate enable
		TH0=0x00; TL0=0x00;
		TR0=1;		//timer0 run enabled
		trig=0;
		while(!INT0);
		while (INT0);
		TR0=0;
		timer_val=TH0<<8|TL0;
		if(timer_val<35000)
			target_range=timer_val*0.01860775;
		else
			target_range=0; // indicates that there is no obstacle in front of the sensor
		
		display(target_range);
		data_to_lcd('c');
		data_to_lcd('m');
		
}
	
void display(unsigned int target_range){
	char array[10];
	int rem,j=0,i=0,m=6;

	while(target_range!=0){
		rem=target_range%10;
		array[i]=rem+'0';
		target_range=target_range/10;
		i++;
	}

	for(j=i-1;j>=0;j--){
		data_to_lcd(array[j]);
		bt_data[m]=array[j];
		m++;
	}
	
}

void initial_delay(){
	TH0=0xB9;
	TL0=0xB0;
	TR0=1;
	while(!TF0);
	TR0=0;
	TF0=0;
}



void cmd_to_lcd(unsigned char cmd){ //lcdcmd function for LCD command{
	P2=cmd;            //P2=lcd command or P2=cmd
	RS=0;                 //LCD command mode
	EN=1;                  //LCD enable=1
	MSdelay(1);           //call MSdelay with 1 ms
	EN=0;                  //LCD enable=0
	return;
}

void data_to_lcd(unsigned char Data){    //lcddata function for LCD Data{
	P2=Data;           //P2=lcd Data
	RS=1;                 //LCD Data mode
	EN=1;                  //LCD enable=1
	MSdelay(1);           //call MSdelay with 1 ms
	EN=0;                  //LCD enable=0 (disable)
	return;
}

void MSdelay(unsigned int time){ //MSdelay function for generating delay
	unsigned int i, j;
	for(i=0; i<time; i++)
	for(j=0; j<1275; j++);
}
