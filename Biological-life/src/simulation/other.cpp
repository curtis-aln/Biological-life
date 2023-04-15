#include "Simulation.hpp"

#include <nlohmann/json.hpp>
#include <fstream> // for std::ofstream

Simulation::Simulation(const Settings& settings)
	: Settings(settings),
	ZoomManagement(m_simBounds, scaleFactor),
	m_buffer(maxCells + maxPlants, objectCirclePoints),
	grid(m_simBounds, hashCells),
	m_allCells(maxCells),
	m_allPlants(maxPlants)
{

	// changing the border to be one spatial cell inwards, this improves cashe hits as it removes boundary checks from the find() query
	m_border = resizeRect(m_border, grid.m_cellDimensions);
	m_simBounds = resizeRect(m_simBounds, grid.m_cellDimensions);

	initLife();
	initDebuging();
}


void Simulation::initLife()
{
	if (minPlants > maxPlants)
		minPlants = maxPlants;

	m_allCells.reserve(maxCells);
	m_allPlants.reserve(maxPlants);

	m_nearbyCells.reserve(static_cast<unsigned long long>(CollisionCell::cell_capacity) * 9);
	m_nearbyPlants.reserve(static_cast<unsigned long long>(CollisionCell::cell_capacity) * 9);

	createCells();
	createPlants();
	overflowProtection(initCellCount, initPlantCount);
}


void Simulation::initDebuging()
	{
	// debug circle for entity center
	debugCircle.setRadius(.5f * scaleFactor);
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

	// zone to show what is in / out of bounds
	debugSimZone.setPosition({ m_simBounds.left, m_simBounds.top });
	debugSimZone.setSize({ m_simBounds.width, m_simBounds.height });
	debugSimZone.setOutlineThickness(.4f * scaleFactor);
	debugSimZone.setOutlineColor({ 255, 255, 255 });
	debugSimZone.setFillColor({ 0, 0, 0, 0 });

	// direction line to show velocity and displacement
	debugDirectionLine.setFillColor({ 0, 0, 255 });
}


Entity Simulation::createEntity(const sf::Color color, const float radius)
{
	constexpr float buffer = 40.f;
	const sf::Vector2f position = randPosInRect(resizeRect(m_simBounds, { buffer , buffer }));
	Entity entity(m_buffer.add(position, radius), m_simBounds);

	entity.setPosition(position);
	m_buffer.setColor({ entity.indexes }, color);
	return entity;
}


void Simulation::createCells()
{
	for (unsigned int i{ 0 }; i < maxCells; i++)
	{
		Genome genes{};
		m_allCells.emplace(Cell{ createEntity(genes.getColor(), genes.getGenomeRadius()), genes });
	}
}


void Simulation::createPlants()
{
	for (unsigned int i{ 0 }; i < maxPlants; i++)
	{
		const sf::Color color = Plant::generateColor();
		Plant plant{ createEntity(color, PlantSettings::initMass), randfloat(0, 100) };
		plant.color = color;
		m_allPlants.emplace(plant);
	}
}


int Simulation::encodeEntityToId(const int index, const bool type)
{
	/* in order to encode the entities into the spatial hash grid, we have the plants with negative id's, and the cells with positive id's.
	 * we also add / remove 1 from the id to avoid having two entities with the "0" id.
	 * type: False = Plant, True = Cell
	 */
	if (type == true) // cell
		return index + 1;
	return index * -1 - 1; // plant
}


void Simulation::decodeEntityIds(std::vector<Cell*>& nearby_cells, std::vector<Plant*>& nearby_plants, const c_Vec& nearbyIds)
{
	for (int32_t i{ 0 }; i < nearbyIds.size; i++)
	{
		if (const int32_t id = nearbyIds.array[i]; id > 0)
			nearby_cells.emplace_back(&m_allCells.at(id - 1));

		else if (id < 0)
			nearby_plants.emplace_back(&m_allPlants.at(id * -1 - 1));
	}
}


template<class E>
void Simulation::overflowCheckEntities(o_vector<E>& entities, const unsigned int maxEntities, const bool isCell) {
	while (entities.size() > maxEntities)
	{
		int randSubIndex = 0;
		if (0 - (static_cast<int>(entities.size()) - 1) != 0)
			randSubIndex = randint(0, static_cast<int>(entities.size()) - 1);

		const unsigned int randIndex = entities.getAvalableIndexes().at(randSubIndex);
		E& entity = entities.at(randIndex);
		removeEntity(entity, randIndex, isCell);
	}
}


void Simulation::overflowProtection(const unsigned int maxcells, const unsigned int maxplants)
{
	overflowCheckEntities(m_allCells, maxcells, true);
	overflowCheckEntities(m_allPlants, maxplants, false);
}


void Simulation::plantUnderflowProtection(const unsigned int minplants)
{
	while (m_allPlants.size() < minplants)
	{
		Plant* plant = m_allPlants.add();
		plant->createRandom();
		m_buffer.setVertexPositions({ plant->indexes }, plant->getDeltaPos());
	}
}


void Simulation::extinctionCheck()
{
	// validating that we should proceed
	if (!autoExtinctionReset || m_allCells.size() > 0 || maxCells == 0 || initCellCount == 0)
		return;

	// correctly re-sizing all entities
	plantUnderflowProtection(initPlantCount);
	overflowCheckEntities(m_allPlants, initPlantCount, false);

	for (unsigned int i{0}; i < initCellCount; i++)
	{
		Cell* cell = m_allCells.add();

		const sf::Vector2f chosenPosition = randPosInRect(m_simBounds);
		const sf::Vector2f deltaPosition = chosenPosition - cell->getPosition();
		cell->setPosition(chosenPosition);
		cell->createRandomMutations(*cell);

		bufferPosUpdate({ cell->indexes }, deltaPosition);
		bufferColorUpdate({ cell->indexes }, cell->getColor());
	}

	totalExtinctions++;
	relativeFrameCount = 0;
}


void Simulation::reSizeSimulation(const unsigned int deltaPixels)
{
	// re-sizing the spatial hash grid
	//grid.reSize();
}


void Simulation::bufferPosUpdate(const Allocations& entityAllocations, const sf::Vector2f deltaPos)
{
	m_buffer.setVertexPositions(entityAllocations, deltaPos);
}

void Simulation::bufferColorUpdate(const Allocations& entityAllocations, const sf::Color newColor)
{
	m_buffer.setColor(entityAllocations, newColor);
}


void Simulation::saveData()
{
	nlohmann::json entityData;
	for (const unsigned index : m_allCells.getAvalableIndexes())
		entityData.push_back(m_allCells.at(index).saveCellJson());

	for (const unsigned index : m_allPlants.getAvalableIndexes())
		entityData.push_back(m_allPlants.at(index).savePlantJson());



	const nlohmann::json simulationData = {
		{"total frame count", totalFrameCount},
		{"total extinctions", totalExtinctions},
		{"total run time", totalRunTime},
		{"min speed", min_speed},
		{"entities", entityData}
	};

	std::ofstream ofs(fileReadWriteName);
	ofs << simulationData.dump(3); // 4 is the number of spaces for indentation
	ofs.close();
}
