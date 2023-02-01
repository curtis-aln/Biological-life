#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct OrganicSettings
{
	const unsigned int initialFoodCount;
	const unsigned int initialLifeCount;
	const unsigned int FrameRate;

	const bool autoExtinctionReset;


};


struct renderingSettings
{
	const unsigned int screenWidth;
	const unsigned int screenHeight;

	const sf::Color windowColor;

	std::string simulationName = "Biologial Evolution Simulation";
};