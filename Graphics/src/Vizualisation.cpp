#include "Vizualisation.h"

Vizualisation::Vizualisation(int Width, int Height) :
    m_width(Width), m_height(Height), m_renderer(Width, Height)
{

   
}

void Vizualisation::Render()
{
    m_renderer.Render({0});
}

Vizualisation::~Vizualisation()
{
}
