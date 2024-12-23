#pragma once
#include "Application.h"

class Sandbox : public Application
{
public:
	Sandbox();
private:
};

void RunApplication()
{
	Sandbox app;
	app.Run();
}