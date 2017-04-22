#include "inverted_pendulum.h"

EncoderPendulum::EncoderPendulum(TIM_TypeDef *TIMx, unsigned int numPerRound /*= 2000*/) :
	EncoderTimer(TIMx),
	npr(numPerRound)
{

}

void EncoderPendulum::refresh()
{
	oldDiff = getDiff();
	EncoderTimer::refresh();
	ddiff = getDiff() - oldDiff;
}

float EncoderPendulum::getRadian()
{
	long posTemp = getPos();
	if (posTemp >= 0)
	{
		return getPos() % npr / (float)npr * 2 * M_PI - M_PI;
	}
	else
	{
		return (npr - (-getPos()) % npr) / (float)npr * 2 * M_PI - M_PI;
	}
}

float EncoderPendulum::getRadianDiff()
{
	return getDiff() / (float)npr * 2 * M_PI;
}

float EncoderPendulum::getRadianDDiff()
{
	return ddiff / (float)npr * 2 * M_PI;
}

MotorBeam::MotorBeam(TIM_TypeDef *TIMx, Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm, unsigned int numPerRound /*= 1560*/, int controlTarget /*= Encoder_Motor_Target_Position*/, float refreshInterval /*= 0.005*/) :
	EncoderMotor(TIMx, motorPinA, motorPinB, motorPinPwm,
		controlTarget, refreshInterval),
	npr(numPerRound)
{

}

float MotorBeam::getRadian()
{
	return getPos() / (double)npr * 2 * M_PI;
}

float MotorBeam::getRadianDiff()
{
	return getPosDiff() / (float)npr * 2 * M_PI;
}

void MotorBeam::setRadianDiff(float radian)
{
	setPosDiff(radian / 2 / M_PI*npr);
}

InvertedPendulum::InvertedPendulum(TIM_TypeDef *TIMpendulum, TIM_TypeDef *TIMmotor, Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm, unsigned int nprPendulum /*= 2000*/, unsigned int nprMotor /*= 1560*/, float refreshInterval /*= 0.005*/) :
	encoder(TIMpendulum, nprPendulum),
	motor(TIMmotor, motorPinA, motorPinB, motorPinPwm,
		nprMotor, Encoder_Motor_Target_Position, refreshInterval),
	refreshInt(refreshInterval),
	enRadThres(M_PI / 4),
	invertedPIDEnable(false)
{

}

void InvertedPendulum::begin()
{
	encoder.begin();
	motor.begin(1.8, 1.75, 0.05);
	setInvertedPIDEnable(true);

	//��ʼ���ڸ˽Ƕ�PID
	pendulumRadianPID.setRefreshInterval(refreshInt);
	pendulumRadianPID.setWeights(1.5, 2, 0.006);
	pendulumRadianPID.setOutputLowerLimit(-1);
	pendulumRadianPID.setOutputUpperLimit(1);
	pendulumRadianPID.setDesiredPoint(0);

	//��ʼ���ڸ˽��ٶ�PID
	pendulumPalstancePID.setRefreshInterval(refreshInt);
	pendulumPalstancePID.setWeights(0, 0, 0);
	pendulumPalstancePID.setOutputLowerLimit(-1);
	pendulumPalstancePID.setOutputUpperLimit(1);
	pendulumPalstancePID.setDesiredPoint(0);

	//��ʼ�������Ƕ�PID
	beamRadianPID.setRefreshInterval(refreshInt);
	beamRadianPID.setWeights(0, 0, 0);
	beamRadianPID.setOutputLowerLimit(-1);
	beamRadianPID.setOutputUpperLimit(1);
	beamRadianPID.setDesiredPoint(0);

	//��ʼ���������ٶ�PID
	beamPalstancePID.setRefreshInterval(refreshInt);
	beamPalstancePID.setWeights(0, 0, 0);
	beamPalstancePID.setOutputLowerLimit(-1);
	beamPalstancePID.setOutputUpperLimit(1);
	beamPalstancePID.setDesiredPoint(0);
}

void InvertedPendulum::refresh()
{
	float desiredRadianPendulum = 0;

	//ˢ�±������͵��λ��PID
	encoder.refresh();
	motor.refresh();

	//��ȡ�������ڸ˽Ƕȡ����ٶ�
	float pendulumRadian = getPendulumRadian();
	float pendulumPalstance = getPendulumPalstance();
	float beamRadian = getBeamRadian();
	float beamPalstance = getBeamPalstance();
	if (pendulumRadian < enRadThres && pendulumRadian>-enRadThres //�ڸ˽Ƕ��ڷ�Χ֮��
		&& invertedPIDEnable)//���ʹ�ܵ���PID
	{
		float motorRadianDiff = 0;
		//�����ĸ�PID�����������+-�Ըı���Ʒ���
		motorRadianDiff -= pendulumRadianPID.refresh(pendulumRadian);
		motorRadianDiff -= pendulumPalstancePID.refresh(pendulumPalstance);
		motorRadianDiff -= beamRadianPID.refresh(beamRadian);
		motorRadianDiff -= beamPalstancePID.refresh(beamPalstance);
		//��������Ƕ�����
		//TODO: ȡ�����λ�ÿ��ƣ�������Ӧ�ӳ�
		motor.setRadianDiff(motorRadianDiff);
	}
}

void InvertedPendulum::setInvertedPIDEnable(bool b)
{
	invertedPIDEnable = b;
}

void InvertedPendulum::setEnRadThres(float t)
{
	if (t < M_PI / 2 && t>0)
	{
		enRadThres = t;
	}
}

float InvertedPendulum::getPendulumRadian()
{
	return encoder.getRadian();
}

float InvertedPendulum::getPendulumPalstance()
{
	return encoder.getRadianDiff() / refreshInt;
}

float InvertedPendulum::getPendulumAcceleration()
{
	return encoder.getRadianDDiff() / refreshInt / refreshInt;
}

float InvertedPendulum::getBeamRadian()
{
	return motor.getRadian();
}

float InvertedPendulum::getBeamPalstance()
{
	return motor.getRadianDiff() / refreshInt;
}

