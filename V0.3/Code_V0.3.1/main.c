/*
	Function Generator with STC MCU
	
	GitHub: https://github.com/CreativeLau/Function_Generator_STC
	
	Copyright (c) 2020 Creative Lau (CreativeLauLab@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	
	Version: V0.3.1
	MCU Type: STC15W4K32S4 @24MHz
	YouTube Tutorial: https://youtu.be/c7iSE34SG90
    Instructable: 
    Any question please contact CreativeLauLab@gmail.com
    YouTube Channel: https://www.youtube.com/c/CreativeLau

	Specification:
	Output: Single Channel
	Square Waveform Frequency: 1Hz~2MHz
	Sine Waveform Frequency: 1Hz~10kHz
	Amplitude: VCC, about 5V
	Load ability: Not available
	Display: LCD1602
	Controller: EC11 Encoder

	Interface:
	Bottom left shows the type of waveform(Square/Sine) and output status(ON/OFF)
	F: Frequency
	D: Duty of Square Waveform
	CD: Clock Division Coefficient (For information only)
	P: PWM frequency for generating Sine Waveform (For information only)
	Pt: Number of points for generating Sine Waveform (For information only)

	Operations:
	Single Click Encoder: Switch Frequency and Duty in Square Waveform Interface
	Double Click Encoder: Start/Stop Signal Output
	Long Press Encoder: Switch between Square Waveform/Sine Waveform/Voltage Information
	Rotate Encoder: Adjust Parameters

	Note: 
	2020-05-06 Update
	1. Fix an error in main funciton, cause the EC11 Encoder rotate disorder.
	2. Fix an error of interface display during switch frequency.
	
	���η�����
	���ߣ�����������
	�汾��V0.3.1
	��Ƭ���ͺţ�STC15W4K32S4 @24MHz
	�κ���������ϵCreativeLauLab@gmail.com
	Bվ��Ƶ�̳̣�https://www.bilibili.com/video/BV12k4y197Qu
	���������ģ�ȫ��ͬ�����ڴ����Ĺ�ע��

	���
	�������ͨ��
	������1Hz~2MHz
	���Ҳ���1Hz-10kHz
	������Լ����VCC��5V����
	�����������޴�������
	��ʾ����LCD1602
	���ƣ�EC11������

	���棺
	���½���ʾ����ͼ�꣨����/���Ҳ��������״̬��On/OFF��
	F��Ƶ��
	D������ռ�ձ�
	CD��ʱ�ӷ�Ƶϵ����For information only��
	P�������������Ҳ���PWMƵ�ʣ�For information only��
	Pt�������������Ҳ��ĵ�����For information only��

	������
	���������������������£��л�Ƶ�ʺ�ռ�ձ�
	˫����������������رղ������
	�������������л���������/���Ҳ�����/��ѹ��ʾ����
	��ת�����������ڲ���

	2020-05-06 ����
	1. ����main����ѭ���л������������ת���ҵĴ���
	   ��main������whileѭ���е�Update_Flag=0;�ŵ�ǰ�����㣬���ں�����ڱ�����������תʱ����δ����ж϶�Update_Flag������������޷���ȷ����LCD
	2. �������л�Ƶ��ʱ���������ʾ����
*/

#include <reg51.h>
#include <intrins.h>
#include "lcd1602.h"
#include "wave.h"
#include "settings.h"
#include "delay.h"
#include "config_stc.h"
//#include "uart.h"
//#include "stdio.h"

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef int8
#define int8 char
#endif

#ifndef uint16
#define uint16 unsigned int
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#define TIMER_0 1 //��ʱ��0�ж����
#define INT_1 2	  //��������ת �����ⲿ�ж�
#define INT_0 0	  //���������� �����ⲿ�ж�

uint8 Timer0_Count;
bit Update_Flag = 1;

void main(void)
{
	//LCD Pin
	P1M1 &= 0x00; //����P1��Ϊ׼˫��
	P1M0 &= 0x00; //����P1��Ϊ׼˫��
	P0M1 &= 0x00; //����P0��Ϊ׼˫��
	P0M0 &= 0x00; //����P0��Ϊ׼˫��

	//�ź����Pin
	PWM3 = 0;	   //����PWM3 P4.5�͵�ƽ
	PWM4 = 0;	   //����PWM4 P4.4�͵�ƽ
	P4M1 |= 0x30;  //����P4.4(PWM4_2),4.5(PWM3_2)Ϊ����
	P4M0 &= ~0x30; //����P4.4(PWM4_2),4.5(PWM3_2)Ϊ����

	/* ��������ת�ж�
	   Interrupt for Encoder Rotation */
	IT1 = 0; //�ⲿ�ж�1������ʽ�������غ��½���
	PX1 = 1; //�ⲿ�ж�1�����ȼ�
	EX1 = 1; //�����ⲿ�ж�1

	/* �����������ж�
	   Interrupt for Encoder Click */
	IT0 = 1; //�ⲿ�ж�0������ʽ���½���
	PX0 = 1; //�ⲿ�ж�0�����ȼ�
	EX0 = 1; //�����ⲿ�ж�0

	/* ��ʱ��0�����ڸ��µ�ѹ��Ϣ��ʱ
	   Timer 0 for updating the information of VCC*/
	TMOD &= 0xF0;  //���ö�ʱ��0ģʽ 16λ�Զ����أ���Keil��debug�Ļ�����ע�⣬����������8051�ľ�13λģʽ
	AUXR &= ~0x80; //��ʱ��0ʱ��12Tģʽ
	TL0 = 0xC0;	   //���ö�ʱ��ֵ 24MHz 20ms
	TH0 = 0x63;	   //���ö�ʱ��ֵ 24MHz 20ms
	ET0 = 1;	   //����T0����ж�

	/* ��ʱ��1����������С��50Hz��PWM
	   Timer 1 for generate the PWM when frequency less than 50Hz*/
	TMOD &= 0x0F;  //����ģʽ,0: 16λ�Զ���װ
	AUXR &= ~0x40; //12T
	ET1 = 1;	   //�����ж�

	EA = 1; //�����ж�

	//UartInit();
	//UartInit_interrupt();
	PWM_Hz_Pre = PWM_Hz;
	Wave_Shape_Pre = Wave_Shape;
	Get_PWM_Duty_Limit();
    if (PWM_Duty > PWM_Max_Duty)
        PWM_Duty = PWM_Max_Duty;
    else if (PWM_Duty < PWM_Min_Duty)
        PWM_Duty = PWM_Min_Duty;
	Lcd_Init();
	while (1)
	{
		if (Update_Flag)
		{
			/*	Update_FlagҪ�������㣬�������Update_LCD���棬����ɼ���Update_LCD�Ĺ������ٴδ�����������ת�жϵĻ���
				��ִ����Update_LCD�����ж�����λ��Update_Flagȴ�������ˣ����LCDûˢ�£��������ʾ�Ĳ�һ�¡�
				��һ�ַ�������ִ��Update_LCDǰ���жϹص���ִ�����ٴ��ж�, �����������ĺ���ִ��ʱ��Ƚϳ����������ת��������ʱ��ʹ�ø��ܿ��١�
				���ж��������ظ��������кô��ģ����Ա����ں���ִ�й����ж�δ����������޸ĵ���
				����ִ�й����в������޸ģ����ܻ���ɼ��������ң�����ʱ����������������Ƭ����λ������	*/
			Update_Flag = 0;
			Wave_OFF();
			//EX1 = 0;
			Update_LCD();
			Set_Wave_Shape();
			//IE1=0;
			//EX1 = 1;
		}
	}
}

/* ��������ת��Ӧ����
   Encoder Rotate */
void Scan_EC11(void)
{
	/* ��ת
	   Rotate clockwise */
	if ((EC11_A != EC11_B))
	{

		Change_Val(1);
	}
	/* ��ת
	   Rotate anticlockwise*/
	else if ((EC11_A == EC11_B))
	{
		Change_Val(0);
	}
}

/* ��������ת�ж�
   Interrupt for Encoder rotation */
void INT1_interrupt(void) interrupt INT_1
{
	Delay1ms();
	Scan_EC11();
	Update_Flag = 1;
	//Delay50ms();
	IE1 = 0;
}

/* ����������ж�
   Interrupt for Encoder click */
void INT0_interrupt(void) interrupt INT_0
{
	Delay5ms();
	if (!EC11_KEY)
	{
		/* ����
		   Long Press */
		if (Delay500ms_long_click())
		{
			Wave_Shape++;
			if (Wave_Shape > WAVE_NUM)
				Wave_Shape = 0;
			if (Wave_Shape == 2)
				Options = 1;
			WAVE_ON = 0;
			Clear_LCD_Flag = 1;
		}
		/* ˫��
		   Double click */
		else if (Delay200ms_double_click())
		{
			if (Wave_Shape > 0)
			{
				WAVE_ON = ~WAVE_ON;
			}
		}
		/* ����
		   Single click */
		else
		{
			if (Wave_Shape == 1)
				Options = ~Options;
		}
		Update_Flag = 1;
	}
	Delay5ms();
	IE0 = 0;
}

/* ���µ�ѹ��Ϣ��ʱ�ж�
   Timer interrupt for update voltage information */
void TIMER0_interrupt() interrupt TIMER_0
{
	if (++Timer0_Count > 200) //200x20=4000ms
	{
		Timer0_Count = 0;
		Update_Flag = 1;
	}
}