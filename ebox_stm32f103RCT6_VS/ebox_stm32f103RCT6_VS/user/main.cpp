/**
  ******************************************************************************
  * @file   : *.cpp
  * @author : shentq
  * @version: V1.2
  * @date   : 2016/08/14

  * @brief   ebox application example .
  *
  * Copyright 2016 shentq. All Rights Reserved.         
  ******************************************************************************
 */

#include "ebox.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "led.h"
#include "encoder_timer.h"
#include "encoder_motor.h"
#include "PID.hpp"
#include <math.h>

#define PID_REFRESH_INTERVAL 0.005
#define M_PI		3.14159265358979323846

Led led1(&PC13, 1);


class EncoderPendulum :public EncoderTimer
{
	unsigned int npr;//ÿȦpos������

public:

	//numPerRound: ÿȦpos������
	EncoderPendulum(TIM_TypeDef *TIMx, 
		unsigned int numPerRound = 2000) :
		EncoderTimer(TIMx),
		npr(numPerRound)
	{}

	////�Գ�ʼ����Ϊ+-180�ȵ�ľ��ԽǶ�ֵ����Χ-180~180����λ��
	//float getDegree()
	//{
	//	return getPos() % npr / (float)npr * 360 - 180;
	//}
	
	//�Գ�ʼ����Ϊ+-pi��ľ��Ի���ֵ����Χ-pi~pi����λ����
	float getRadian()
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
};

class MotorBeam :public EncoderMotor
{
	unsigned int npr;//ÿȦpos������

public:

	//numPerRound: ÿȦpos������
	MotorBeam(TIM_TypeDef *TIMx,
		Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
		unsigned int numPerRound = 1560,
		int controlTarget = Encoder_Motor_Target_Position,
		float refreshInterval = 0.005):
		EncoderMotor(TIMx, motorPinA, motorPinB, motorPinPwm,
			controlTarget, refreshInterval),
		npr(numPerRound)
	{}

	////�Գ�ʼ����Ϊ0�ȵ�ľ��ԽǶ�ֵ����Χ-nan~+nan����λ��
	//double getDegree()
	//{
	//	return getPos() / (double)npr * 360;
	//}

	//�Գ�ʼ����Ϊ0���ȵ�ľ��ԽǶ�ֵ����Χ-nan~+nan����λ����
	double getRadian()
	{
		return getPos() / (double)npr * 2 * M_PI;
	}

	//����Ŀ��ǶȻ��Ȳ�
	void setRadianDiff(float radian)
	{
		setPosDiff(radian / 2 / M_PI*npr);
	}
};

class InvertedPendulum
{

	greg::PID pendulumPID, beamPID;
	float refreshInt;
	const float enableRadian;//����pid�����ĽǶȷ�Χ�������򣬵�λ����
	bool enableInvertedPID;
public:
	EncoderPendulum encoder;
	MotorBeam motor;

	InvertedPendulum(TIM_TypeDef *TIMpendulum,TIM_TypeDef *TIMmotor,
		Gpio *motorPinA, Gpio *motorPinB, Gpio *motorPinPwm,
		unsigned int nprPendulum = 2000, unsigned int nprMotor=1560, float refreshInterval = 0.005) :
		encoder(TIMpendulum, nprPendulum),
		motor(TIMmotor, motorPinA, motorPinB, motorPinPwm, 
			nprMotor, Encoder_Motor_Target_Position, refreshInterval),
		refreshInt(refreshInterval),
		enableRadian(M_PI / 4),
		enableInvertedPID(false)
	{}

	void begin()
	{
		encoder.begin();
		motor.begin(1.8, 1.75, 0.05);
		setEnableInvertedPID(true);

		//TODO: �����Ƕ����ơ�pid���Σ�������ֵ��λ�ĳɻ���
		//��ʼ���ڸ˽Ƕ�PID
		pendulumPID.setRefreshInterval(refreshInt);
		pendulumPID.setWeights(1.5, 2, 0.006);
		pendulumPID.setOutputLowerLimit(-2);
		pendulumPID.setOutputUpperLimit(2);
		pendulumPID.setDesiredPoint(0);

		//��ʼ������λ��PID
		beamPID.setRefreshInterval(refreshInt);
		//beamPID.setWeights(0.018, 0, 0.006);
		beamPID.setWeights(0, 0, 0);
		beamPID.setOutputLowerLimit(-0.5);
		beamPID.setOutputUpperLimit(0.5);
		beamPID.setDesiredPoint(0);
	}

	void refresh()
	{
		float desiredRadianPendulum = 0;

		//ˢ�±������͵��λ��PID
		encoder.refresh();
		motor.refresh();

		float radianPendulum = -encoder.getRadian();
		if (radianPendulum < enableRadian && radianPendulum>-enableRadian && enableInvertedPID)
		{
			//����λ��
			desiredRadianPendulum = beamPID.refresh(atan(-motor.getRadian()));
			pendulumPID.setDesiredPoint(desiredRadianPendulum);
			//�ڸ˽Ƕ�PID
			motor.setRadianDiff(pendulumPID.refresh(tan(radianPendulum)));
		}
	}

	void setEnableInvertedPID(bool b)
	{
		enableInvertedPID = b;
	}
};

InvertedPendulum invertedPendulum(TIM4, TIM3,
	&PA2, &PA1, &PA0,
	2000, 1560, PID_REFRESH_INTERVAL);

//float motorRadian = 0;
static void vDebugTask(void *pvParameters)
{
	while (1)
	{
		led1.toggle();
		vTaskDelay(10 / portTICK_RATE_MS);
		//uart1.printf("%ld\t\t%lf\t\t%ld\t\t%f\r\n",
		//	invertedPendulum.motor.getPos(),
		//	invertedPendulum.motor.getRadian(),
		//	invertedPendulum.encoder.getPos(),
		//	invertedPendulum.encoder.getRadian()
		//	);
		//motorRadian = invertedPendulum.motor.getRadian();
	}
}


static void vPIDTask(void *pvParameters)
{
	while (1)
	{
		vTaskDelay(PID_REFRESH_INTERVAL*1000 / portTICK_RATE_MS);
		invertedPendulum.refresh();
	}
}


void setup()
{
    ebox_init();
    uart1.begin(115200);
	led1.begin();

	invertedPendulum.begin();
	//invertedPendulum.setEnableInvertedPID(false);
	//invertedPendulum.motor.setRadianDiff(0.5);

	//����RTOS����
	set_systick_user_event_per_sec(configTICK_RATE_HZ);
	attach_systick_user_event(xPortSysTickHandler);

	xTaskCreate(vDebugTask, "Debug", configMINIMAL_STACK_SIZE, (void *)0, NULL, NULL);
	xTaskCreate(vPIDTask, "PID", configMINIMAL_STACK_SIZE, (void *)0, NULL, NULL);
	vTaskStartScheduler();
}

int main(void)
{
    setup();

    while(1)
    {
    }
}


