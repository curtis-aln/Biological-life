#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>

#include "../SpatialHashGrid/spatialHashGrid.h"
#include "../Life/cell.hpp"
#include "../Life/plant.hpp"
#include "o_vector.hpp"
#include "zooming.hpp"

#include <string>


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
	// ---------- borders and boundaries ---------- //
	sf::Rect<float> m_border{ 0, 0, windowSize.x, windowSize.y }; // window space

	// in the simulation we will slowly change the simbounds to a more scarce environment
	sf::Rect<float> m_DesiredBounds{ 0, 0, windowSize.x / scaleFactor, windowSize.y / scaleFactor };
	sf::Rect<float> m_simBounds = resizeRect(m_DesiredBounds, { sim_init_buffer, sim_init_buffer });


	// ---------- spatial hash grid ---------- //
	sf::Vector2u hashCells = {
		static_cast<unsigned>(static_cast<float>(hashGridCells.x) / scaleFactor),
		static_cast<unsigned>(static_cast<float>(hashGridCells.y) / scaleFactor) };
	SpatialHashGrid m_hashGrid{};

	// ---------- SFML window ---------- //
	sf::Clock m_clock{};
	sf::RenderWindow m_window{sf::VideoMode(
		static_cast<unsigned>(windowSize.x), static_cast<unsigned>(windowSize.y)), simulationName};

	// ---------- Vertex Buffer ---------- //
	Buffer m_buffer;

	// ---------- containers ---------- //
	o_vector<Cell, maxCells>   m_Cells{};
	o_vector<Plant, maxPlants> m_Plants{};

	std::vector<Cell*>  m_nearbyCells{};
	std::vector<Plant*> m_nearbyPlants{};

	// ---------- debugging ---------- //
	sf::CircleShape debugCircle{};
	sf::CircleShape debugVRange{};
	sf::CircleShape debugCircleSize{};

	bool m_debugVRangeToggle  = false;
	bool m_debugCircToggle    = false;
	bool m_debugCenterToggle  = false;
	bool m_debugVelToggle     = false;
	bool m_debugClosestToggle = false;
	bool m_debugBorder        = false;

	// ---------- runtime variables ---------- //
	bool m_paused       = false;
	bool m_drawGrid     = false;
	bool m_closeSim     = false;
	bool m_autoSaving   = false;
	bool m_frameByFrame = false;
	bool m_thermal      = false;


	// ---------- other statistics ---------- //
	unsigned long long totalFrameCount = 0;
	unsigned long long relativeFrameCount = 0;
	unsigned           totalExtinctions = 0;
	unsigned           updateCounter = 0;
	double             totalRunTime = 0;

	std::vector<unsigned> cellPopulation{ };
	std::vector<unsigned> plantPopulation{ };
	std::vector<float>    avgReproCount{ };
	std::vector<float>    avgLifeTime{ };


	// ---------- camera movement ---------- //
	bool m_mousePressed = false;



public:
	explicit Simulation(const Settings& settings);
	void run();


private: // physics
	void tickFrame();
	void endFrame(double deltaTime);
	void alignCells();
	void prepGrid();

	void initStatisticVariables();
	void saveData();
	void loadData();
	void clearEntityData();

	void updatePlants();
	void bufferPosUpdate(const Allocations& entityAllocations, sf::Vector2f deltaPos);
	void bufferColorUpdate(const Allocations& entityAllocations, sf::Color newColor);
	
	void prepareCells();
	void updateCells();

	template<class E>
	void removeEntity(E* entity, bool type);

	template<class E, unsigned N>
	void alignEntites(o_vector<E, N>& entities);

	template <class E, unsigned N>
	void updateEntityPosition(o_vector<E, N>& entities);

	template <class E, unsigned N>
	void addAndRemoveEntities(o_vector<E, N>& entities, bool isCell);

	template<class E, unsigned N>
	bool addEntity(o_vector<E, N>& entities, E* entity, const bool isCell);



private: // rendering
	void pollEvents();
	void printStatistics();
	void updateStatistics();
	void updateCellStatistics();
	void keyPressEvents(const sf::Keyboard::Key& event_key_code);
	void renderFrame();

	void debugEntities();
	void debugEntity(const Entity* entity, float vrange, float initRad);


private: // other
	void initLife();
	void initDebuging();

	Entity createEntity(sf::Color color, float radius);
	void createCells();
	void createPlants();

	static int encodeEntityToId(unsigned index, bool type);
	void decodeEntityIds(std::vector<Cell*>& nearby_cells, std::vector<Plant*>& nearby_plants, const c_Vec&
	                     nearbyIds);

	template <class E, unsigned N>
	void overflowCheckEntities(o_vector<E, N>& entities, unsigned maxEntities, const bool type);

	void overflowProtection(unsigned maxcells, unsigned maxplants);
	void plantUnderflowProtection(unsigned minplants);
	void extinctionCheck();
};
