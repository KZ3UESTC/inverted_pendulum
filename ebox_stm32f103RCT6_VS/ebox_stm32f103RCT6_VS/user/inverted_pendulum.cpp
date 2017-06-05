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




void InvertedPendulum::workDisabled()
{
	motor.setPercent(0);
	pendulumRadian = getPendulumRadian();
}

bool InvertedPendulum::conditionDisabledToSwingBegin(int event)
{
	return (event == Inverted_Pendulum_Mode_Swing || event == Inverted_Pendulum_Mode_SwingInvert);
}

bool InvertedPendulum::conditionDisabledToInvert(int event)
{
	return (event == Inverted_Pendulum_Mode_Invert
		&& (pendulumRadian < enRadThres && pendulumRadian>-enRadThres));
}

bool InvertedPendulum::conditionDisabledToRound(int event)
{
	return (event == Inverted_Pendulum_Mode_Round
		&& (pendulumRadian < enRadThres && pendulumRadian>-enRadThres));
}

void InvertedPendulum::workSwingBegin()
{
	motor.setPercent(50);
	pendulumRadian = getPendulumRadian();
}

bool InvertedPendulum::conditionSwingBeginToSwing(int event)
{
	return ((event == Inverted_Pendulum_Mode_Swing || event == Inverted_Pendulum_Mode_SwingInvert)
		&& (pendulumRadian < PI / 1.5 && pendulumRadian>-PI / 1.5));
}

bool InvertedPendulum::conditionSwingBeginToDisabled(int event)
{
	return (event == Inverted_Pendulum_Mode_Disabled);
}

void InvertedPendulum::entryInvert()
{
	resetPID();
	setTargetBeamRadian(getBeamRadian());
	setTargetBeamPalstance(0);
}

void InvertedPendulum::workInvert()
{
	refreshPID();
	pendulumRadian = getPendulumRadian();
}

bool InvertedPendulum::conditionInvertToDisabled(int event)
{
	return ((event == Inverted_Pendulum_Mode_Invert
		&& (pendulumRadian >= enRadThres || pendulumRadian <= -enRadThres))
		|| event == Inverted_Pendulum_Mode_Disabled);
}

bool InvertedPendulum::conditionInvertToRound(int event)
{
	return (event == Inverted_Pendulum_Mode_Round);
}

bool InvertedPendulum::conditionInvertToSwing(int event)
{
	return (event == Inverted_Pendulum_Mode_SwingInvert
		&& (pendulumRadian >= enRadThres || pendulumRadian <= -enRadThres));
}

void InvertedPendulum::entryRound()
{
	resetPID();
	setTargetBeamRadian(getBeamRadian());
	setTargetBeamPalstance(2);
}

void InvertedPendulum::workRound()
{
	pendulumRadian = getPendulumRadian();

	//���ú���Ŀ���ٶ�
	//�Ժ���ʵ�ʽǶ���Ŀ��ǶȽ����ж�
	if (abs(getBeamRadian() - getTargetBeamRadian()) < PI)
	{
		setTargetBeamRadian(getTargetBeamRadian() + targetBeamPalstance*refreshInt);
	}
	else
	{
		if (getBeamRadian() - getTargetBeamRadian() > 0)
		{
			setTargetBeamRadian(getBeamRadian() - PI);
		}
		else
		{
			setTargetBeamRadian(getBeamRadian() + PI);
		}
	}

	refreshPID();
}

bool InvertedPendulum::conditionRoundToInvert(int event)
{
	return (event == Inverted_Pendulum_Mode_Invert);
}

bool InvertedPendulum::conditionRoundToDisabled(int event)
{
	return ((event == Inverted_Pendulum_Mode_Round
		&& (pendulumRadian >= enRadThres || pendulumRadian <= -enRadThres))
		|| event == Inverted_Pendulum_Mode_Disabled);
}

void InvertedPendulum::workSwing()
{
	refreshSwing();
	pendulumRadian = getPendulumRadian();
}

////�ν�����ĩβ��state��ʼ����ʽ������ע���ʼ��˳���ǰ�����˳�򣡣���
//InvertedPendulum::InvertedPendulum(TIM_TypeDef *TIMpendulum,
//	TIM_TypeDef *TIMmotor, Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
//	unsigned int nprPendulum /*= 2000*/, unsigned int nprMotor /*= 1560*/, float refreshInterval /*= 0.005*/) :
//	encoder(TIMpendulum, nprPendulum),
//	motor(TIMmotor, motorPinA, motorPinB, motorPinPwm,
//		nprMotor, Encoder_Motor_PID_Disabled, refreshInterval),
//	refreshInt(refreshInterval),
//	enRadThres(PI / 3),
//	mode(Inverted_Pendulum_Mode_Disabled),
//	targetBeamPalstance(0),
//	targetBeamRadian(0),
//	//��ʼ������������
//	fsm(&stateDisabled),
//	//��ʼ��stateDisabled��ת���б�
//	transDisabledToSwingBegin(this, &InvertedPendulum::conditionDisabledToSwingBegin, &stateSwingBegin),
//	transDisabledToInvert(this, &InvertedPendulum::conditionDisabledToInvert, &stateInvert, &transDisabledToSwingBegin),
//	transDisabledToRound(this, &InvertedPendulum::conditionDisabledToRound, &stateRound, &transDisabledToInvert),
//	stateDisabled(this, &InvertedPendulum::workDisabled, NULL, NULL, &transDisabledToSwingBegin),
//	//��ʼ��stateSwingBegin��ת���б�
//	transSwingBeginToSwing(this, &InvertedPendulum::conditionSwingBeginToSwing, &stateSwing),
//	transSwingBeginToDisabled(this, &InvertedPendulum::conditionSwingToDisabled, &stateDisabled, &transSwingBeginToSwing),
//	stateSwingBegin(this, &InvertedPendulum::workSwingBegin, NULL, NULL, &transSwingBeginToSwing),
//	//��ʼ��stateInvert��ת���б�
//	transInvertToDisabled(this, &InvertedPendulum::conditionInvertToDisabled, &stateDisabled),
//	transInvertToRound(this, &InvertedPendulum::conditionInvertToRound, &stateRound, &transInvertToDisabled),
//	transInvertToSwing(this, &InvertedPendulum::conditionInvertToSwing, &stateSwing, &transInvertToRound),
//	stateInvert(this, &InvertedPendulum::workInvert, &InvertedPendulum::entryInvert, NULL, &transInvertToDisabled),
//	//��ʼ��stateRound��ת���б�
//	transRoundToInvert(this, &InvertedPendulum::conditionRoundToInvert, &stateInvert),
//	transRoundToDisabled(this, &InvertedPendulum::conditionRoundToDisabled, &stateDisabled, &transRoundToInvert),
//	stateRound(this, &InvertedPendulum::workRound, &InvertedPendulum::entryRound, NULL, &transRoundToInvert),
//	//��ʼ��stateSwing��ת���б�
//	transSwingToDisabled(this, &InvertedPendulum::conditionSwingToDisabled, &stateDisabled),
//	transSwingToSwingBegin(this, &InvertedPendulum::conditionSwingToSwingBegin, &stateSwingBegin, &transSwingToDisabled),
//	transSwingToInvert(this, &InvertedPendulum::conditionSwingToInvert, &stateInvert, &transSwingToSwingBegin),
//	stateSwing(this, &InvertedPendulum::workSwing, NULL, NULL, &transSwingToDisabled)
//{
//
//}

InvertedPendulum::InvertedPendulum(TIM_TypeDef *TIMpendulum,
	TIM_TypeDef *TIMmotor, Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
	unsigned int nprPendulum /*= 2000*/, unsigned int nprMotor /*= 1560*/, float refreshInterval /*= 0.005*/) :
	encoder(TIMpendulum, nprPendulum),
	motor(TIMmotor, motorPinA, motorPinB, motorPinPwm,
		nprMotor, Encoder_Motor_PID_Disabled, refreshInterval),
	refreshInt(refreshInterval),
	enRadThres(PI / 3),
	mode(Inverted_Pendulum_Mode_Disabled),
	targetBeamPalstance(0),
	targetBeamRadian(0),
	//��ʼ������������
	fsm(&stateDisabled),
	//��ʼ��stateDisabled��ת���б�
	transDisabledToSwingBegin(this, &InvertedPendulum::conditionDisabledToSwingBegin, &stateSwingBegin),
	transDisabledToInvert(this, &InvertedPendulum::conditionDisabledToInvert, &stateInvert),
	transDisabledToRound(this, &InvertedPendulum::conditionDisabledToRound, &stateRound),
	stateDisabled(this, &InvertedPendulum::workDisabled, NULL, NULL),
	//��ʼ��stateSwingBegin��ת���б�
	transSwingBeginToSwing(this, &InvertedPendulum::conditionSwingBeginToSwing, &stateSwing),
	transSwingBeginToDisabled(this, &InvertedPendulum::conditionSwingToDisabled, &stateDisabled),
	stateSwingBegin(this, &InvertedPendulum::workSwingBegin, NULL, NULL),
	//��ʼ��stateInvert��ת���б�
	transInvertToDisabled(this, &InvertedPendulum::conditionInvertToDisabled, &stateDisabled),
	transInvertToRound(this, &InvertedPendulum::conditionInvertToRound, &stateRound),
	transInvertToSwing(this, &InvertedPendulum::conditionInvertToSwing, &stateSwing),
	stateInvert(this, &InvertedPendulum::workInvert, &InvertedPendulum::entryInvert, NULL),
	//��ʼ��stateRound��ת���б�
	transRoundToInvert(this, &InvertedPendulum::conditionRoundToInvert, &stateInvert),
	transRoundToDisabled(this, &InvertedPendulum::conditionRoundToDisabled, &stateDisabled),
	stateRound(this, &InvertedPendulum::workRound, &InvertedPendulum::entryRound, NULL),
	//��ʼ��stateSwing��ת���б�
	transSwingToDisabled(this, &InvertedPendulum::conditionSwingToDisabled, &stateDisabled),
	transSwingToSwingBegin(this, &InvertedPendulum::conditionSwingToSwingBegin, &stateSwingBegin),
	transSwingToInvert(this, &InvertedPendulum::conditionSwingToInvert, &stateInvert),
	stateSwing(this, &InvertedPendulum::workSwing, NULL, NULL)
{
	//��addTransItem�������ת���б�����ÿ�������˳��
	stateDisabled.addTransItem(&transDisabledToSwingBegin);
	stateDisabled.addTransItem(&transDisabledToInvert);
	stateDisabled.addTransItem(&transDisabledToRound);
	stateSwingBegin.addTransItem(&transSwingBeginToSwing);
	stateSwingBegin.addTransItem(&transSwingBeginToDisabled);
	stateInvert.addTransItem(&transInvertToSwing);
	stateInvert.addTransItem(&transInvertToRound);
	stateInvert.addTransItem(&transInvertToDisabled);
	stateRound.addTransItem(&transRoundToDisabled);
	stateRound.addTransItem(&transRoundToInvert);
	stateSwing.addTransItem(&transSwingToDisabled);
	stateSwing.addTransItem(&transSwingToSwingBegin);
	stateSwing.addTransItem(&transSwingToInvert);
}

void InvertedPendulum::begin()
{
	encoder.begin();
	motor.begin();

	//��ʼ���ڸ˽Ƕ�PID
	pendulumRadianPID.setRefreshInterval(refreshInt);
	pendulumRadianPID.setWeights(700, 1400, 0);
	pendulumRadianPID.setOutputLowerLimit(-INF_FLOAT);
	pendulumRadianPID.setOutputUpperLimit(INF_FLOAT);
	pendulumRadianPID.setDesiredPoint(0);

	//��ʼ���ڸ˽��ٶ�PID
	pendulumPalstancePID.setRefreshInterval(refreshInt);
	pendulumPalstancePID.setWeights(10, 0, 0);
	pendulumPalstancePID.setOutputLowerLimit(-INF_FLOAT);
	pendulumPalstancePID.setOutputUpperLimit(INF_FLOAT);
	pendulumPalstancePID.setDesiredPoint(0);

	//��ʼ�������Ƕ�PID
	beamRadianPID.setRefreshInterval(refreshInt);
	//beamRadianPID.setWeights(200, 100, 0);//��Ӧ��PID
	//beamRadianPID.setWeights(60, 40, 0);//������PID
	beamRadianPID.setWeights(60, 40, 0);
	beamRadianPID.setOutputLowerLimit(-INF_FLOAT);
	beamRadianPID.setOutputUpperLimit(INF_FLOAT);
	beamRadianPID.setDesiredPoint(0);

	//��ʼ���������ٶ�PID
	beamPalstancePID.setRefreshInterval(refreshInt);
	//beamPalstancePID.setWeights(6, 10, 0);//��Ӧ��PID
	//beamPalstancePID.setWeights(1, 0.1, 0);//������PID
	beamPalstancePID.setWeights(3, 6, 0);
	beamPalstancePID.setOutputLowerLimit(-INF_FLOAT);
	beamPalstancePID.setOutputUpperLimit(INF_FLOAT);
	beamPalstancePID.setDesiredPoint(0);
}

void InvertedPendulum::refresh()
{
	//ˢ�±������͵��λ��PID
	encoder.refresh();
	motor.refresh();
	pendulumRadian = getPendulumRadian();
	//ˢ��״̬��
	fsm.refresh(mode);
}


bool InvertedPendulum::conditionSwingToDisabled(int event)
{
	if (event == Inverted_Pendulum_Mode_Disabled)
	{
		return true;
	}
	return false;
}

bool InvertedPendulum::conditionSwingToSwingBegin(int event)
{
	if ((event == Inverted_Pendulum_Mode_Swing
		|| event == Inverted_Pendulum_Mode_SwingInvert)
		&& ((pendulumRadian > PI*0.8 || pendulumRadian < -PI*0.8)
			&& abs(getBeamPalstance()) < 0.1))
	{
		return true;
	}
	return false;
}

bool InvertedPendulum::conditionSwingToInvert(int event)
{
	if (event == Inverted_Pendulum_Mode_SwingInvert
		&& pendulumRadian < PI / 8 && pendulumRadian>-PI / 8)
	{
		return true;
	}
	return false;
}

void InvertedPendulum::refreshPID()
{
	float motorPercent = 0;

	//��ȡ�������ڸ˽Ƕȡ����ٶ�
	float pendulumRadian = getPendulumRadian();
	float pendulumPalstance = getPendulumPalstance();
	float beamRadian = getBeamRadian();
	float beamPalstance = getBeamPalstance();

	//�����ĸ�PID�����������+-�Ըı���Ʒ���
	motorPercent -= pendulumRadianPID.refresh(pendulumRadian);
	motorPercent -= pendulumPalstancePID.refresh(pendulumPalstance);
	motorPercent -= beamRadianPID.refresh(beamRadian);
	motorPercent -= beamPalstancePID.refresh(beamPalstance);

	//��������Ƕ�����
	motor.setPercent(motorPercent);
}

void InvertedPendulum::refreshSwing()
{
	float motorPercent = 0;
	float pendulumAcceleration = getPendulumAcceleration();
	//�������������
	motorPercent -= 0.5*pendulumAcceleration;
	//�޷�
	if (motorPercent>50)
	{
		motorPercent = 50;
	}
	else if (motorPercent<-50)
	{
		motorPercent = -50;
	}
	//��������Ƕ�����
	motor.setPercent(motorPercent);
}

void InvertedPendulum::setMode(Inverted_Pendulum_Mode_Typedef m)
{
	mode = m;
}


int InvertedPendulum::getMode()
{
	return mode;
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

void InvertedPendulum::resetPID()
{
	pendulumRadianPID.reset();
	beamRadianPID.reset();
	pendulumPalstancePID.reset();
	beamPalstancePID.reset();

	pendulumRadianPID.setDesiredPoint(0);
	setTargetBeamRadian(0);
	pendulumPalstancePID.setDesiredPoint(0);
	beamPalstancePID.setDesiredPoint(0);
}

void InvertedPendulum::setTargetBeamPalstance(float desiredBeamPalstance)
{
	targetBeamPalstance = desiredBeamPalstance;
	beamPalstancePID.setDesiredPoint(desiredBeamPalstance);
}

void InvertedPendulum::setTargetBeamRadian(float desiredBeamRadian)
{
	targetBeamRadian = desiredBeamRadian;
	beamRadianPID.setDesiredPoint(desiredBeamRadian);
}

float InvertedPendulum::getTargetBeamRadian()
{
	return targetBeamRadian;
}

