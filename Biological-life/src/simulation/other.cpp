#include "Simulation.hpp"

#include <nlohmann/json.hpp>
#include <fstream> // for std::ofstream


Simulation::Simulation(const Settings& settings)
	: Settings(settings),
	ZoomManagement(m_simBounds, scaleFactor),
	m_hashGrid(m_DesiredBounds, hashCells),
	m_buffer(maxCells + maxPlants, objectCirclePoints)
{
	// changing the border to be one spatial cell inwards, this improves cashe hits as it removes boundary checks from the find() query
	m_border = resizeRect(m_border, m_hashGrid.m_cellDimensions);
	m_simBounds = resizeRect(m_simBounds, m_hashGrid.m_cellDimensions);

	initStatisticVariables();
	initLife();
	initDebuging();
}


void Simulation::initLife()
{

	if (minPlants > maxPlants)
		minPlants = maxPlants;

	m_nearbyCells.reserve(static_cast<unsigned long long>(CollisionCell::cell_capacity) * 9);
	m_nearbyPlants.reserve(static_cast<unsigned long long>(CollisionCell::cell_capacity) * 9);

	createCells();
	createPlants();
	std::cout << "entities initilised" << "\n";
	overflowProtection(initCellCount, initPlantCount);
	std::cout << "initial overflow complete" << "\n";
}


void Simulation::initDebuging()
	{
	// debug circle for entity center
	debugCircle.setRadius(2.f * scaleFactor);
	debugCircle.setFillColor({ 255, 0, 0 });

	// circle used for entity visual range
	debugVRange.setRadius(PlantSettings::visualRange);
	debugVRange.setOutlineColor({ 255, 0, 0 });
	debugVRange.setOutlineThickness(.6f * scaleFactor);
	debugVRange.setFillColor({ 0, 0, 0, 0 });

	// circle used for entity size
	debugCircleSize.setRadius(PlantSettings::initMass);
	debugCircleSize.setOutlineColor({ 255, 0, 0 });
	debugCircleSize.setOutlineThickness(.4f * scaleFactor);
	debugCircleSize.setFillColor({ 0, 0, 0, 0 });
}


Entity Simulation::createEntity(const sf::Color color, const float radius)
{
	const sf::Vector2f position = randPosInRect(resizeRect(m_simBounds, m_hashGrid.m_cellDimensions));
	Entity entity(m_buffer.add(position, radius), &m_simBounds, color, radius);

	entity.setEntityPosition(position);
	m_buffer.setColor({ entity.indexes }, color);
	return entity;
}


void Simulation::createCells()
{
	for (unsigned i{ 0 }; i < maxCells; i++)
	{
		const Cell cell{ createEntity(Genome::randCellColor(), PlantSettings::initMass + 4), Genome(), i};
		m_Cells.emplace(cell);
	}
}   


void Simulation::createPlants()
{
	for (unsigned i{ 0 }; i < maxPlants; i++)
	{
		const sf::Color color = Plant::generateColor();
		const Plant plant{ createEntity(color, PlantSettings::initMass), randfloat(0, 100), i };
		m_Plants.emplace(plant);
	}
}


int Simulation::encodeEntityToId(const unsigned index, const bool type)
{
	/* encoding entities into an integer value, cells (type = true) > 0, plants (type = false) < 0 */
	if (type == true) return static_cast<int>(index + 1);
	return static_cast<int>(index * -1 - 1);
}


void Simulation::decodeEntityIds(std::vector<Cell*>& nearby_cells, std::vector<Plant*>& nearby_plants, const c_Vec& nearbyIds)
{
	for (int32_t i{ 0 }; i < nearbyIds.size; i++)
	{
		if (const int32_t id = nearbyIds.array[i]; id > 0)
			nearby_cells.emplace_back(m_Cells.at(id - 1));

		else if (id < 0)
			nearby_plants.emplace_back(m_Plants.at(id * -1 - 1));
	}
}




void Simulation::plantUnderflowProtection(const unsigned minplants)
{
	while (m_Plants.size() < minplants)
	{
		Plant* plant = m_Plants.add();
		plant->createRandom();
		m_buffer.setVertexPositions({ plant->indexes }, plant->getDeltaPos());
	}
}



void Simulation::bufferPosUpdate(const Allocations& entityAllocations, const sf::Vector2f deltaPos)
{
	m_buffer.setVertexPositions(entityAllocations, deltaPos);
}

void Simulation::bufferColorUpdate(const Allocations& entityAllocations, const sf::Color newColor)
{
	m_buffer.setColor(entityAllocations, newColor);
}