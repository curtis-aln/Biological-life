#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>

#include "settings.hpp"

struct DeltaTime
{
	DeltaTime()
	{
		m_start = std::chrono::high_resolution_clock::now();
	}

	double GetDelta()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		const auto delta = currentTime - m_start;
		m_start = currentTime;
		return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(delta).count());
	}

private:
	std::chrono::high_resolution_clock::time_point m_start;
};



struct Renderer : renderingSettings
{
	sf::RenderWindow m_window{};

	bool m_paused = false;
	bool closeSim = false;


protected:
	void pollEvents()
	{
		sf::Event event{};
		while (m_window.pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
			{
				keyPressEvents(event.key.code);
			}
		}
	}

	void keyPressEvents(const sf::Keyboard::Key& event_key_code)
	{
		switch (event_key_code)
		{
		case sf::Keyboard::Escape:
			closeSim = true;
			break;

		case sf::Keyboard::Space:
			m_paused = not m_paused;
			break;

		default:
			break;

		}
	}


public:

	explicit Renderer(const renderingSettings& settings) : renderingSettings(settings),
	         m_window(sf::RenderWindow(sf::VideoMode(screenWidth, screenHeight), simulationName, sf::Style::None)) {}
	~Renderer();


	void render()
	{
		m_window.clear(windowColor);

		pollEvents();

		m_window.display();
	}

};

inline Renderer::~Renderer() = default;


class Simulation : OrganicSettings, Renderer, DeltaTime
{
	unsigned long long totalFrameCount = 0;
	unsigned long long totalRunTime = 0;



public:
	explicit Simulation(const OrganicSettings& organicSettings, const renderingSettings& renderSettings)
				: OrganicSettings(organicSettings), Renderer(renderSettings), DeltaTime() {}
	~Simulation() = default;


	void run()
	{
		while (closeSim == false)
		{
			tick(GetDelta());
		}
	}


private:
	void tick(const double deltaTime)
	{
		// ticking the simulation

		// rendering the simulation
		render();

		// updating runtime statistics
		totalFrameCount++;
		totalRunTime += static_cast<long long>(deltaTime);
	}
};
