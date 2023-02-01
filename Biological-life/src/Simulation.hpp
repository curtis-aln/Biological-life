#pragma once

#include <SFML/Graphics.hpp>
#include "settings.hpp"


struct Renderer : renderingSettings
{
	sf::RenderWindow m_window{};

	bool m_paused = false;


private:


public:

	explicit Renderer(const renderingSettings& settings) : renderingSettings(settings)
	{
		// initilising window
		m_window.setSize({screenWidth, screenHeight});
	}
	~Renderer();


	void render()
	{
		
	}

};

inline Renderer::~Renderer()
{
	// any necessary cleanup code here
}



class Simulation : OrganicSettings, Renderer
{
	unsigned long long frameCount = 0;

public:
	explicit Simulation(const OrganicSettings& organicSettings, const renderingSettings& renderSettings)
				: OrganicSettings(organicSettings), Renderer(renderSettings) {}
	~Simulation() = default;
};
