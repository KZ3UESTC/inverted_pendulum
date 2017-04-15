#pragma once

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "ebox.h"

//δ��ӳ�䶨ʱ��ch1��ch2�������
//- TIM1 : PA8 PA9
//- TIM2 : PA0 PA1
//- TIM3 : PA6 PA7
//- TIM4 : PB6 PB7
class EncoderTimer
{
	long pos;
	short diff;
	Gpio *pinA;
	Gpio *pinB;
	TIM_TypeDef *timer;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
public:
	EncoderTimer(TIM_TypeDef *TIMx);

	//����IO�ͼĴ���
	void begin();

	//��ȡλ��
	long getPos();

	//����λ��
	void resetPos();

	//��ȡtimer�Ĵ��������
	void refresh();

	//��ȡ�ٶ�
	short getDiff();
};