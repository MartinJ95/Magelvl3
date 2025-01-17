#include "Vizualisation.h"

Vizualisation::Vizualisation(int Width, int Height) :
    m_width(Width), m_height(Height), m_renderer(Width, Height)
{

   
}

void Vizualisation::AddToRenderQueue(const Vector3& position)
{
}

void Vizualisation::Render()
{
    while (!m_renderer.WindowShouldClose())
    {
        m_renderer.PollEvents();
        m_renderer.Render();
    }
}

Vizualisation::~Vizualisation()
{
}
