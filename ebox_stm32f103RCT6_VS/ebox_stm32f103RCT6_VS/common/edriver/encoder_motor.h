#ifndef  __ENCODER_MOTOR
#define  __ENCODER_MOTOR

#include "ebox.h"
#include "PID.hpp"
#include "encoder_timer.h"
#include "tb6612fng.h"

typedef enum
{
	Encoder_Motor_Target_Position,
	Encoder_Motor_Target_Speed
}Encoder_Motor_Target_Typedef;


//δ��ӳ�䶨ʱ��ch1��ch2�������
//- TIM1 : PA8 PA9
//- TIM2 : PA0 PA1
//- TIM3 : PA6 PA7
//- TIM4 : PB6 PB7
class EncoderMotor
{
	EncoderTimer encoder;
	TB6612FNG driver;
	int mode;
	float percent;
public:
	greg::PID pid;

	EncoderMotor(TIM_TypeDef *TIMx,
		Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
		int controlTarget = Encoder_Motor_Target_Position,
		float refreshInterval = 0.01);

	//���ñ�����������
	void begin();

	//���ñ�������������������PID
	void begin(const float &Kp, const float &Ki, const float &Kd);

	//ˢ�±�����������PID�����
	void refresh();

	//��ȡλ��
	long getPos();

	//��ȡ�ٶ�
	short getSpd();

	//��ȡ����ٷֱ�
	float getPercent();

	//����Ŀ��λ�ã�λ�ÿ���ģʽ��
	void setPos(long pos);

	//����Ŀ��λ��������λ�ÿ���ģʽ��
	void setPosDiff(int pos);

	//����Ŀ���ٶȣ��ٶȿ���ģʽ��
	void setSpd(long spd);
};

#endif
