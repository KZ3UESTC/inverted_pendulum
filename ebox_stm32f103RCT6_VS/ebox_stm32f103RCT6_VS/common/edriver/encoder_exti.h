#ifndef  __ENCODER_EXTI
#define  __ENCODER_EXTI

#include "ebox.h"

class EncoderExti
{
	long long position;
	long long positionOld;
	long difference;
	Gpio *a_pin;
	Gpio *b_pin;
	Exti extiA;
	Exti extiB;
	void eventA();
	void eventB();
public:

	//��ʼ��������������Apin��Bpin�ֱ�Ϊ��������A��B��
	EncoderExti(Gpio *Apin, Gpio *Bpin);

	//��ʼ����eboxѡ��
	void begin();

	//��ȡλ��
	long long getPosition();

	//����λ��Ϊ0
	void resetPosition();

	//�������ϴ�λ�õĲ�ֵ
	void countDiff();

	//��ȡ����Ĳ�ֵ
	long getDiff();
};




#endif