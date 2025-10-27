#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include "TFixedArray.h"

constexpr int SubscribeAmount = 10;

template<typename T>
class Subject;

template<typename T>
class Observer
{
public:
	Observer()
	{

	}
	void AddSubject(Subject<T>* NewSubject)
	{
		//m_subject = NewSubject;
		m_subject.Add(NewSubject);
	}
	virtual void OnNotify(const T&) = 0;
	void SetSubjects(std::vector<Subject<T>*> NewSubjects)
	{
		//m_subject = NewSubjects;
	}
private:
	TFixedArray<Subject<T>*, SubscribeAmount> m_subject{nullptr};
};

template<typename T>
class Subject
{
public:
	Subject()
	{
	}
	Subject(const Subject& Other) : 
		m_observers(Other.m_observers)
	{}
	Subject(Subject&& Other) :
		m_observers(std::move(Other.m_observers))
	{
		std::cout << "hey" << std::endl;
	}
public:
	virtual void Notify(const T& Data)
	{
		for (int i = 0; i < m_observers.Size(); i++)
		{
			if (m_observers[i] == nullptr)
			{
				continue;
			}
			m_observers[i]->OnNotify(Data);
		}
		/*
		for (auto& it : m_observers)
		{
			if (it == nullptr)
			{
				continue;
			}
			it->OnNotify(Data);
			
		}
		*/
	}
	void AddSubscriber(Observer<T>* NewObserver)
	{
		m_observers.Add(NewObserver);
		NewObserver->AddSubject(this);
	}
	void SetObservers(std::vector<Observer<T>*> NewObservers)
	{
		//m_observers = NewObservers;
	}
	void operator=(const Subject& Other)
	{
		m_observers = Other.m_observers;
	}
	void operator=(Subject&& Other)
	{
		m_observers = std::move(Other.m_observers);
	}
private:
	TFixedArray<Observer<T>*, SubscribeAmount> m_observers{nullptr};
};