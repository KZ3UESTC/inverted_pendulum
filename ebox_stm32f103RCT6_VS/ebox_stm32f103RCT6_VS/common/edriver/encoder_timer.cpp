#include "encoder_timer.h"

EncoderTimer::EncoderTimer(TIM_TypeDef *TIMx) :
	pos(0),diff(0)
{
	timer = TIMx;

	if (timer == TIM1)
	{
		pinA = &PA8;
		pinB = &PA9;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	}
	if (timer == TIM2)
	{
		pinA = &PA0;
		pinB = &PA1;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	}
	if (timer == TIM3)
	{
		pinA = &PA6;
		pinB = &PA7;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	}
	if (timer == TIM4)
	{
		pinA = &PB6;
		pinB = &PB7;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	}
}

void EncoderTimer::begin()
{
	pinA->mode(INPUT);
	pinB->mode(INPUT);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // Ԥ��Ƶ�� 
	TIM_TimeBaseStructure.TIM_Period = (u16)(65535); //�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//ѡ��ʱ�ӷ�Ƶ������Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM���ϼ���  
	TIM_TimeBaseInit(timer, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(timer, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//ʹ�ñ�����ģʽ3
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(timer, &TIM_ICInitStructure);
	TIM_ClearFlag(timer, TIM_FLAG_Update);//���TIM�ĸ��±�־λ
	TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
	//Reset counter
	TIM_SetCounter(timer, 0);
	TIM_Cmd(timer, ENABLE);
}

long EncoderTimer::getPos()
{
	return pos;
}

void EncoderTimer::resetPos()
{
	pos = 0;
}

void EncoderTimer::refresh()
{
	diff = timer->CNT;
	timer->CNT = 0;
	pos = pos + diff;
}

short EncoderTimer::getDiff()
{
	return diff;
}