#ifndef __FSM_H
#define __FSM_H

#include "FunctionPointer.h"

//״̬ת���б���
class FiniteStateMachineState;
class FiniteStateMachineTrans
{
	FunctionPointerArg1<bool, int> _condition;
	FiniteStateMachineState* targetState;
	FiniteStateMachineTrans* next;
public:
	friend class FiniteStateMachineState;
	friend class FiniteStateMachine;

	//ʵ����һ��TransItem��
	//event��ת������-�¼�
	//condition��ת������-�����б�ʽ
	//targetState��Ŀ��״̬
	//before���������һ��TransItem
	FiniteStateMachineTrans(bool(*condition)(int), FiniteStateMachineState* targetState, FiniteStateMachineTrans* before = NULL) :
		targetState(targetState),
		next(NULL)
	{
		if (before != NULL)
		{
			before->next = this;
		}
		_condition.attach(condition);
	}

	//ʵ����һ��TransItem��
	//event��ת������-�¼�
	//classPointer�����condition�����ǳ�Ա��������Ҫ���ָ��
	//condition��ת������-�����б�ʽ
	//targetState��Ŀ��״̬
	//before���������һ��TransItem
	template<typename T>
	FiniteStateMachineTrans(T* classPointer, bool(T::*condition)(int), FiniteStateMachineState* targetState, FiniteStateMachineTrans* before = NULL) :
		targetState(targetState),
		next(NULL)
	{
		if (before != NULL)
		{
			before->next = this;
		}
		_condition.attach(classPointer, condition);
	}
	bool condition(int event)
	{
		return _condition.call(event);
	}
};

class FiniteStateMachineState
{
	FunctionPointer _entry;
	FunctionPointer _work;
	FunctionPointer _exit;
	//����ת���б��������ʽ�洢
	FiniteStateMachineTrans *transList, *transEnd;//ά��һ��transEndָ�룬������ĩβ�����
	friend class FiniteStateMachine;

	void refreshTransEnd()
	{
		if (transList != NULL)
		{
			FiniteStateMachineTrans *nowTransItem = transList;
			while (1)
			{
				if (nowTransItem->next != NULL)
				{
					nowTransItem = nowTransItem->next;
				}
				else
				{
					break;
				}
			}
			transEnd = nowTransItem;
		}
		else
		{
			transEnd = NULL;
		}
	}
public:

	//����һ������ͨ������Ϊ��Ա��״̬
	FiniteStateMachineState(void(*work)(void), void(*entry)(void), void(*exit)(void), FiniteStateMachineTrans* transList = NULL) :
		_work(work),
		_entry(entry),
		_exit(exit),
		transList(transList),
		transEnd(transList)
	{
		refreshTransEnd();
	}

	//����һ���Գ�Ա������Ϊ��Ա��״̬
	template<typename T>
	FiniteStateMachineState(T* classPointer, void(T::*work)(void), void(T::*entry)(void), void(T::*exit)(void), FiniteStateMachineTrans* transList = NULL) :
		_work(classPointer, work),
		_entry(classPointer, entry),
		_exit(classPointer, exit),
		transList(transList),
		transEnd(transList)
	{
		refreshTransEnd();
	}

	//����һ���Գ�Ա������Ϊ��Ա��״̬
	template<typename T>
	FiniteStateMachineState(T* classPointer, void(T::*work)(void), int empty1, int empty2, FiniteStateMachineTrans* transList = NULL) :
		_work(classPointer, work),
		transList(transList),
		transEnd(transList)
	{

	}

	//����һ���Գ�Ա������Ϊ��Ա��״̬
	template<typename T>
	FiniteStateMachineState(T* classPointer, void(T::*work)(void), int empty1, void(T::*exit)(void), FiniteStateMachineTrans* transList = NULL) :
		_work(classPointer, work),
		_exit(classPointer, exit),
		transList(transList),
		transEnd(transList)
	{
		refreshTransEnd();
	}

	//����һ���Գ�Ա������Ϊ��Ա��״̬
	template<typename T>
	FiniteStateMachineState(T* classPointer, void(T::*work)(void), void(T::*entry)(void), int empty2, FiniteStateMachineTrans* transList = NULL) :
		_work(classPointer, work),
		_entry(classPointer, entry),
		transList(transList),
		transEnd(transList)
	{
		refreshTransEnd();
	}


	//Ϊ״̬�������ת�Ʊ�
	void setTransList(FiniteStateMachineTrans* transList)
	{
		this->transList = transList;
	}

	//Ϊ����ת�Ʊ���ӱ���
	void addTransItem(FiniteStateMachineTrans* transItem)
	{
		if (transEnd == NULL)
		{
			transList = transItem;
			transEnd = transItem;
		}
		else
		{
			transEnd->next = transItem;
			transEnd = transItem;
		}
	}

	void entry()
	{
		_entry.call();
	}

	void work()
	{
		_work.call();
	}

	void exit()
	{
		_exit.call();
	}

};

//����״̬��
//1. ��������״̬��
//2. ������״̬
//3. ������״̬��״̬ת�Ʊ�
//4. ���õ�ǰ״̬
class FiniteStateMachine
{
	FiniteStateMachineState* activeState;
public:

	//��������״̬��
	FiniteStateMachine(FiniteStateMachineState* activeState = NULL) :
		activeState(activeState)
	{
		if (activeState != NULL)
		{
			activeState->entry();
		}
	}

	//���õ�ǰ״̬
	//����״̬֮ǰִ����һ��State��exit����
	//��ִ����һ��State��entry����
	//����ÿ��refresh��ִ��work
	void setActivaState(FiniteStateMachineState* activeState)
	{
		//����״̬֮ǰִ����һ��State��exit����
		this->activeState->exit();

		this->activeState = activeState;

		//��ִ����һ��State��entry����
		this->activeState->entry();
	}

	void refresh(int event)
	{
		if (activeState != NULL)
		{
			this->activeState->work();

			//����ת���б�
			if (activeState->transList != NULL)//ת���б�ǿ�
			{
				FiniteStateMachineTrans* nowItem = activeState->transList;
				while (1)
				{
					//���㵱ǰ�¼���Event���뵱ǰ״̬������ת���б����¼���ͬ
					//����״̬������ת���б����¼���Ӧ��bool�б�ʽ��������
					if (nowItem->condition(event))
					{
						setActivaState(nowItem->targetState);
						break;
					}
					nowItem = nowItem->next;
					if (nowItem == NULL)
					{
						break;
					}
				}
			}
		}
	}
};


#endif

