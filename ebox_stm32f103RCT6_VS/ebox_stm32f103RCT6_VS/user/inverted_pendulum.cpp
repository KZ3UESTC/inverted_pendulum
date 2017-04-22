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
		return getPos() % npr / (float)npr * 2 * PI - PI;
	}
	else
	{
		return (npr - (-getPos()) % npr) / (float)npr * 2 * PI - PI;
	}
}

float EncoderPendulum::getRadianDiff()
{
	return getDiff() / (float)npr * 2 * PI;
}

float EncoderPendulum::getRadianDDiff()
{
	return ddiff / (float)npr * 2 * PI;
}

MotorBeam::MotorBeam(TIM_TypeDef *TIMx, Gpio *motorPinA, Gpio *motorPinB,
	Gpio *motorPinPwm, unsigned int numPerRound /*= 1560*/,
	Encoder_Motor_Target_Typedef controlTarget /*= Encoder_Motor_Target_Position*/, float refreshInterval /*= 0.005*/) :
	EncoderMotor(TIMx, motorPinA, motorPinB, motorPinPwm,
		controlTarget, refreshInterval),
	npr(numPerRound)
{

}

float MotorBeam::getRadian()
{
	return getPos() / (double)npr * 2 * PI;
}

float MotorBeam::getRadianDiff()
{
	return getPosDiff() / (float)npr * 2 * PI;
}

void MotorBeam::setRadianDiff(float radian)
{
	setPosDiff(radian / 2 / PI*npr);
}

InvertedPendulum::InvertedPendulum(TIM_TypeDef *TIMpendulum, 
	TIM_TypeDef *TIMmotor, Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
	unsigned int nprPendulum /*= 2000*/, unsigned int nprMotor /*= 1560*/, float refreshInterval /*= 0.005*/) :
	encoder(TIMpendulum, nprPendulum),
	motor(TIMmotor, motorPinA, motorPinB, motorPinPwm,
		nprMotor, Encoder_Motor_Target_Position, refreshInterval),
	refreshInt(refreshInterval),
	enRadThres(PI / 3),
	mode(Inverted_Pendulum_Mode_Disabled),
	beamPalstance(0)
{

}

void InvertedPendulum::begin()
{
	encoder.begin();
	motor.begin(5, 0, 0.06);
	setMode(Inverted_Pendulum_Mode_Invert);

	//��ʼ���ڸ˽Ƕ�PID
	pendulumRadianPID.setRefreshInterval(refreshInt);
	pendulumRadianPID.setWeights(0.5, 1.2, 0);
	//pendulumRadianPID.setWeights(0.5, 1.2, 0.0018);
	pendulumRadianPID.setOutputLowerLimit(-INF_FLOAT);
	pendulumRadianPID.setOutputUpperLimit(INF_FLOAT);
	pendulumRadianPID.setDesiredPoint(0);

	//��ʼ���ڸ˽��ٶ�PID
	pendulumPalstancePID.setRefreshInterval(refreshInt);
	pendulumPalstancePID.setWeights(0.001, 0.0005, 0);
	pendulumPalstancePID.setOutputLowerLimit(-INF_FLOAT);
	pendulumPalstancePID.setOutputUpperLimit(INF_FLOAT);
	pendulumPalstancePID.setDesiredPoint(0);

	//��ʼ�������Ƕ�PID
	beamRadianPID.setRefreshInterval(refreshInt);
	beamRadianPID.setWeights(0.08, 0.05, 0);
	beamRadianPID.setOutputLowerLimit(-INF_FLOAT);
	beamRadianPID.setOutputUpperLimit(INF_FLOAT);
	beamRadianPID.setDesiredPoint(0);

	//��ʼ���������ٶ�PID
	beamPalstancePID.setRefreshInterval(refreshInt);
	beamPalstancePID.setWeights(0.006, 0.0004, 0);
	beamPalstancePID.setOutputLowerLimit(-INF_FLOAT);
	beamPalstancePID.setOutputUpperLimit(INF_FLOAT);
	beamPalstancePID.setDesiredPoint(0);
}

void InvertedPendulum::refresh()
{
	float desiredRadianPendulum = 0;

	//ˢ�±������͵��λ��PID
	encoder.refresh();
	motor.refresh();

	if (mode != Inverted_Pendulum_Mode_Disabled)
	{
		float motorRadianDiff = 0;
		float pendulumRadian = getPendulumRadian();
		//�ڸ˵���PID�ļ���ͷ���
		if (mode == Inverted_Pendulum_Mode_Invert//���ʹ�ܵ���PID
			&& pendulumRadian < enRadThres && pendulumRadian>-enRadThres)//�ڸ˽Ƕ��ڷ�Χ֮��
		{
			beamRadianPID.setDesiredPoint(getBeamRadian() + beamPalstance*refreshInt);
			beamPalstancePID.setDesiredPoint(beamPalstance);
			//��ȡ�������ڸ˽Ƕȡ����ٶ�
			float pendulumPalstance = getPendulumPalstance();
			float beamRadian = getBeamRadian();
			float beamPalstance = getBeamPalstance();
			//�����ĸ�PID�����������+-�Ըı���Ʒ���
			motorRadianDiff -= pendulumRadianPID.refresh(pendulumRadian);
			motorRadianDiff -= pendulumPalstancePID.refresh(pendulumPalstance);
			motorRadianDiff -= beamRadianPID.refresh(beamRadian);
			motorRadianDiff -= beamPalstancePID.refresh(beamPalstance);
			//��������Ƕ�����
			//TODO: ȡ�����λ�ÿ��ƣ�������Ӧ�ӳ�
			motor.setRadianDiff(motorRadianDiff);
		}
		else if (mode == Inverted_Pendulum_Mode_Swing)
		{
			//�������������
			motorRadianDiff += 1.0 * getPendulumAcceleration();
			//��������Ƕ�����
			//TODO: ȡ�����λ�ÿ��ƣ�������Ӧ�ӳ�
			motor.setRadianDiff(motorRadianDiff);
		}
		//��������Ƕȿ��Ʒ�Χ��resetPID
		if (pendulumRadian >= enRadThres || pendulumRadian<=-enRadThres)
		{
			resetInvertPID();
			beamRadianPID.setDesiredPoint(getBeamRadian());
		}
	}
}


void InvertedPendulum::setEnRadThres(float t)
{
	if (t < PI / 2 && t>0)
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

void InvertedPendulum::resetInvertPID()
{
	pendulumRadianPID.reset();
	beamRadianPID.reset();
	pendulumPalstancePID.reset();
	beamPalstancePID.reset();

	pendulumRadianPID.setDesiredPoint(0);
	beamRadianPID.setDesiredPoint(0);
	pendulumPalstancePID.setDesiredPoint(0);
	beamPalstancePID.setDesiredPoint(0);
}

void InvertedPendulum::setBeamPalstance(float targetBeamPalstance)
{
	beamPalstance = targetBeamPalstance;
}

