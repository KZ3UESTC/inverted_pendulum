#ifndef __FSM
#define __FSM

//�Ժ�����Ϊ״̬������״̬��
class FSM
{
	//��ǰ״̬
	void (*activeState)();

public:
	//���ó�ʼ״̬
	FSM(void(*startState)());

	//���õ�ǰ״̬������״̬��ת
	void setActiveState(void (*state)());

	//ˢ�£�ִ�е�ǰ״̬����
	void refresh();
};


#endif