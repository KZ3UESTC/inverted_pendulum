#ifndef INVERTED_PENDULUM
#define INVERTED_PENDULUM

#include "encoder_timer.h"
#include "encoder_motor.h"
#include "PID.hpp"

#define PID_REFRESH_INTERVAL 0.005
#define M_PI		3.14159265358979323846


class EncoderPendulum :public EncoderTimer
{
	unsigned int npr;//ÿȦpos������

public:

	//��������ת��������Ϊ���Ա�����ʹ��
	//numPerRound: ÿȦpos������
	EncoderPendulum(TIM_TypeDef *TIMx,
		unsigned int numPerRound = 2000);

	//��ȡ���Ի���ֵ���Գ�ʼ����Ϊ+-pi�㣬��Χ-pi~pi
	float getRadian();

	//��ȡ���ٶȣ���λ����
	float getRadianDiff();
};

class MotorBeam :public EncoderMotor
{
	unsigned int npr;//ÿȦpos������

public:

	//������������Ի�ȡ�Ի���Ϊ��λ����ԽǶ�ֵ
	//numPerRound: ÿȦpos������
	MotorBeam(TIM_TypeDef *TIMx,
		Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
		unsigned int numPerRound = 1560,
		int controlTarget = Encoder_Motor_Target_Position,
		float refreshInterval = 0.005);

	//��ȡ����ֵ���Գ�ʼ����Ϊ0���ȵ㣬��Χ-nan~+nan
	float getRadian();

	//��ȡ���ٶȣ���λ����
	float getRadianDiff();

	//����Ŀ��ǶȻ��Ȳ�
	void setRadianDiff(float radian);
};

class InvertedPendulum
{
	float refreshInt;
	float enRadThres;//����pid�����ĽǶȷ�Χ�������򣬵�λ���ȡ���ʼpi/4
	bool invertedPIDEnable;
public:
	greg::PID pendulumRadianPID, beamRadianPID,//�Ƕ�PID
		pendulumPalstancePID, beamPalstancePID;//���ٶ�PID
	EncoderPendulum encoder;
	MotorBeam motor;

	InvertedPendulum(TIM_TypeDef *TIMpendulum, TIM_TypeDef *TIMmotor,
		Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
		unsigned int nprPendulum = 2000, unsigned int nprMotor = 1560, float refreshInterval = 0.005);

	//��ʼ���������������PID���ƣ����ó�ʼ��PIDֵ
	void begin();

	//�Ա����������PID������PID����ˢ��
	void refresh();

	//���õ���PIDʹ��
	void setInvertedPIDEnable(bool b);

	//���ý���pid�����ĽǶȷ�Χ
	void setEnRadThres(float t);
};

#endif