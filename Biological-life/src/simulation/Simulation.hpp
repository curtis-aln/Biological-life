#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>

#include "../SpatialHashGrid/spatialHashGrid.h"
#include "../Life/cell.hpp"
#include "../Life/Plant.hpp"
#include "o_vector.hpp"
#include "zooming.hpp"

#include <string>


struct Settings
{
	// organic simulation settings
	unsigned int initPlantCount;
	unsigned int initCellCount;
	const unsigned int FrameRate;

	const bool autoExtinctionReset;

	// graphical settings
	sf::Vector2f windowSize;
	const float scaleFactor;

	// buffer settings
	const unsigned int maxCells;
	unsigned int maxPlants;
	const unsigned int objectCirclePoints;

	unsigned int minPlants;

	const sf::Color windowColor;

	std::string simulationName;
	std::string fileReadWriteName;
	sf::Vector2u hashGridCells;
};


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
		return std::chrono::duration_cast<std::chrono::duration<double>>(delta).count();
	}

private:
	std::chrono::high_resolution_clock::time_point m_start;
};


class Simulation : Settings, DeltaTime, ZoomManagement
{
	sf::Clock m_clock{};

	sf::Rect<float> m_border{ 0, 0, windowSize.x, windowSize.y };
	sf::Rect<float> m_simBounds{ 0, 0, windowSize.x / scaleFactor, windowSize.y / scaleFactor };


	sf::Vector2u hashCells = {
		static_cast<unsigned>(static_cast<float>(hashGridCells.x) / scaleFactor),
		static_cast<unsigned>(static_cast<float>(hashGridCells.y) / scaleFactor) };

	sf::RenderWindow m_window{sf::VideoMode(
		static_cast<unsigned>(windowSize.x), static_cast<unsigned>(windowSize.y)), simulationName};
	Buffer m_buffer;

	SpatialHashGrid grid{};

	o_vector<Cell> m_allCells{};
	o_vector<Plant> m_allPlants{};

	// temporary vectors
	std::vector<Cell*> m_nearbyCells{};
	std::vector<Plant*> m_nearbyPlants{};

	// debugging
	sf::CircleShape debugCircle{};
	sf::CircleShape debugVRange{};
	sf::CircleShape debugCircleSize{};
	sf::RectangleShape debugSimZone{};
	sf::RectangleShape debugDirectionLine{};

	// runtime variables
	bool m_paused = false;
	bool m_drawGrid = false;
	bool m_closeSim = false;
	bool m_autoSaving = false;

	bool m_debugVRangeToggle = false;
	bool m_debugCircToggle = false;
	bool m_debugCenterToggle = false;
	bool m_debugZoneToggle = false;
	bool m_debugVelToggle = false;
	bool m_debugClosestToggle = false;

	// other statistics
	unsigned long long totalFrameCount = 0;
	unsigned long long relativeFrameCount = 0;
	unsigned int       totalExtinctions = 0;
	unsigned int       updateCounter = 0;
	double totalRunTime = 0;

	// camera movement
	bool m_mousePressed = false;

	// natural selection
	float min_speed = 0.005f;



public:
	explicit Simulation(const Settings& settings);
	void run();


private: // physics
	void runFrame(double deltaTime);
	void prepGrid();

	void saveData();

	void updatePlants();
	void bufferPosUpdate(const Allocations& entityAllocations, const sf::Vector2f deltaPos);
	void bufferColorUpdate(const Allocations& entityAllocations, sf::Color newColor);
	
	void updateCells();
	void removeEntity(Entity& entity, unsigned relativeIndex, bool type);

	template<class E>
	void alignEntites(o_vector<E>& entities);

	template <class E>
	void positionAlignment(o_vector<E>& entities);

	template <class E>
	bool addEntity(o_vector<E>& entities, E& entity, bool isCell);

	template <class E>
	void addAndRemoveEntities(o_vector<E>& entities, bool isCell);

	//template <class T>
	//T* filterAndProcessNearby(const sf::Vector2f position, std::vector<T*>& entities, const float visualRange, const float radius);


private: // rendering
	void pollEvents();
	void printStatistics();
	void keyPressEvents(const sf::Keyboard::Key& event_key_code);
	void renderFrame();

	void debugEntities();
	void debugEntity(Entity& entity, const float vrange, const float initRad);


private: // other
	void initLife();
	void initDebuging();

	Entity createEntity(sf::Color color, float radius);
	void createCells();
	void createPlants();

	static int encodeEntityToId(int index, bool type);
	void decodeEntityIds(std::vector<Cell*>& nearby_cells, std::vector<Plant*>& nearby_plants, const c_Vec& nearbyIds);

	template <class E>
	void overflowCheckEntities(o_vector<E>& entities, unsigned maxEntities, bool isCell);

	void overflowProtection(unsigned maxcells, unsigned maxplants);
	void plantUnderflowProtection(unsigned minplants);
	void extinctionCheck();
	void reSizeSimulation(unsigned deltaPixels);
};
