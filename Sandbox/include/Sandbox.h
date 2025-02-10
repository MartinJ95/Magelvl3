#pragma once

#include "Application.h"

class Sandbox : public Application
{
public:
	Sandbox();
	void OnGUI() override final;
private:
};

void RunApplication()
{
	Sandbox app;
	app.Run();
}