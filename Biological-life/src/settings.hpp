#pragma once

#include <SFML/Graphics.hpp>

struct Settings
{
	// organic simulation settings
	const unsigned initPlantCount;
	const unsigned initCellCount;
	const unsigned FrameRate;

	const bool autoExtinctionReset;
	const bool cellCroudingDeath;

	const unsigned alignmentFreq;


	// graphical settings
	sf::Vector2f windowSize;
	const float scaleFactor;
	const float sim_init_buffer;
	const sf::Color windowColor;
	const std::string simulationName;

	// buffer settings
	const unsigned objectCirclePoints;
	unsigned minPlants;

	const std::string fileReadWriteName;
	const sf::Vector2u hashGridCells;

	static constexpr unsigned maxCells = 10'000;
	static constexpr unsigned maxPlants = 4'000;
};


struct CellSettings
{
	static constexpr float visualRadius = 82.f;
	static constexpr float energyTransferRate = 7.f;

	static constexpr int maxTimeAlone = 60;
	static constexpr int reproductionDelay = 40;

	static constexpr int sensoryInputs = 8;
	static constexpr int hiddenLayerSize = 5;
	static constexpr int sensoryOutputs = 5;

	static constexpr int numHiddenLayers = 1;

};


struct PlantSettings
{
	static constexpr float initMass = 8.f;
	static constexpr float visualRange = initMass * 6.5f; // must be <= 1.5 grid cells

protected:
	static constexpr float growSpeed = 0.05f;
	static constexpr float reproMass = 10;
	static constexpr float reproAmount = 3;

	static constexpr unsigned reproAge = 10'000;
	static constexpr unsigned reproMinCollisions = 10;
	static constexpr unsigned randDeathChance = 1000;

	static constexpr float friction = 1.00f;
	static constexpr float maxSpeed = .2f;
	static constexpr float attractStrength = 0.0002f;

	static constexpr float initialPlantEnergy = 50.f;
};


class GenomeSettings
{
protected:

    // scale factors
    inline static constexpr float massScale = 1.0f;
    inline static constexpr float speedScale = 1.f;

    // mutation and gene related (mr = mutation rate)
    inline static constexpr float changeInMR = 0.00'09f;
    inline static constexpr int colorMR = 4;

    // ranges
    inline static const sf::Vector2f massR = { 10.f, 10.f };
    inline static const sf::Vector2f speedR = { 0.0f, 1.f };
    inline static const sf::Vector2f mutationRateR = { 0.00'05f, 0.00'40f };
    inline static const sf::Vector2f geneticCodeR = { -0.02f, 0.06f };
};