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
        m_renderer.AddToRenderQueue(0, Vector3(10, 0, 0));
        m_renderer.AddToRenderQueue(0, Vector3(-10, 0, 0));
        m_renderer.AddToRenderQueue(0, Vector3(0, 10, 0));
        m_renderer.AddToRenderQueue(0, Vector3(0, -10, 0));
        m_renderer.AddToRenderQueue(0, Vector3(0, 0, 10));
        m_renderer.AddToRenderQueue(0, Vector3(0, 0, -10));
        /*
        */
        m_renderer.Render();
    }
}

Vizualisation::~Vizualisation()
{
}
