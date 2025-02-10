#include "Vizualisation.h"

Vizualisation* VizInstance = nullptr;

Renderer* RendererInstance = nullptr;

Vizualisation::Vizualisation(int Width, int Height) :
    m_width(Width), m_height(Height), m_renderer(Width, Height)
{
    
   
}

void Vizualisation::AddToRenderQueue(const Vector3& position)
{
}

void Vizualisation::Render()
{
    m_renderer.PollEvents();
    m_renderer.Render();
}

bool Vizualisation::WindowShouldClose() const
{
    return m_renderer.WindowShouldClose();
}

void Vizualisation::OnGUIStart()
{
    m_renderer.OnGUIStart();
}

Vizualisation::~Vizualisation()
{
}
