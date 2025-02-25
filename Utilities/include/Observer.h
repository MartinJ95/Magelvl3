#pragma once
#include <vector>

template<typename T>
class Subject;

template<typename T>
class Observer
{
public:
	Observer() : m_subject()
	{

	}
	void AddSubject(Subject<T>* NewSubject)
	{
		//m_subject = NewSubject;
		m_subject.emplace_back(NewSubject);
	}
	virtual void OnNotify(const T&) = 0;
private:
	std::vector<Subject<T>*> m_subject;
};

template<typename T>
class Subject
{
public:
	virtual void Notify(const T& Data)
	{
		for (int i = 0; i < m_observers.size(); i++)
		{
			if (m_observers.at(i) == nullptr)
			{
				continue;
			}
			m_observers.at(i)->OnNotify(Data);
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
		m_observers.emplace_back(NewObserver);
		NewObserver->AddSubject(this);
	}
private:
	std::vector<Observer<T>*> m_observers;
};