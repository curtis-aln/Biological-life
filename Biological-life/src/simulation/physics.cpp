#include "Simulation.hpp"
#include "../Life/entity.hpp"

void Simulation::run()
{
	while (!m_closeSim)
	{
		const double deltaTime = GetDelta();

		if (!m_paused)
			tickFrame();
		endFrame(deltaTime);
		renderFrame();
	}
}


void Simulation::tickFrame()
{
	prepGrid();

	addAndRemoveEntities(m_Cells, true);
	addAndRemoveEntities(m_Plants, false);

	updatePlants();
	prepareCells();

	updateCells();

	overflowProtection(maxCells, maxPlants);
	plantUnderflowProtection(minPlants);
	extinctionCheck();


	if (totalFrameCount % alignmentFreq == 0)
		alignEntites(m_Cells);

	m_buffer.update();

}

void Simulation::endFrame(const double deltaTime)
{
	// allows the user to manually tick through frames
	if (m_frameByFrame) m_paused = true;

	// updating runtime statistics and ending the frame
	totalFrameCount++;
	relativeFrameCount++;
	totalRunTime += deltaTime;
	
	// growing the world size to a less dense area
	constexpr float boundarySF = -0.001f;
	if (m_simBounds.left - m_DesiredBounds.left > m_hashGrid.m_cellDimensions.x)
		m_simBounds = resizeRect(m_simBounds, {boundarySF, boundarySF});

	updateStatistics();

}

void Simulation::alignCells()
{
	alignEntites(m_Cells);
}

template<class E, unsigned N>
void Simulation::alignEntites(o_vector<E, N>& entities)
{
	/* entities often fall out of sync with their display and real position so it is nessesery to re-align them every now and then */
	for (E* entity : entities)
	{
		sf::Vector2f& actualCenter = m_buffer.getVertices()->at(entity->indexes[0] + 2).position;
		sf::Vector2f desiredCenter = entity->getPosition();
		sf::Vector2f delta = desiredCenter - actualCenter;

		bufferPosUpdate({ entity->indexes }, delta);
	}
}


void Simulation::prepGrid()
{
	m_hashGrid.clear();

	// first loop is for adding the cells
	for (const Cell* cell : m_Cells)
		m_hashGrid.addAtom(cell->getPosition(), encodeEntityToId(cell->vector_id, true));
	

	// second loop is for adding the plants
	for (const Plant* plant : m_Plants)
		m_hashGrid.addAtom(plant->getPosition(), encodeEntityToId(plant->vector_id, false));
}


void Simulation::updatePlants()
{
	for (Plant* plant : m_Plants)
	{
		m_nearbyCells.clear();
		m_nearbyPlants.clear();
		decodeEntityIds(m_nearbyCells, m_nearbyPlants, m_hashGrid.find(plant->getPosition()));

		plant->update(m_nearbyPlants);
	}

	updateEntityPosition(m_Plants);
}


void Simulation::prepareCells()
{
	// this iteration updates all of the positions of the cells
	for (Cell* cell : m_Cells)
	{
		// clearing the recycled containers and filling them with new entities
		m_nearbyCells.clear();
		m_nearbyPlants.clear();
		decodeEntityIds(m_nearbyCells, m_nearbyPlants, m_hashGrid.find(cell->getPosition()));

		// crouding death check
		if (cellCroudingDeath && m_nearbyCells.size() >= c_Vec::max / 4)
			cell->die();

		// getting entity information
		unsigned nearbyCellCount = 0;
		unsigned nearbyPlantCount = 0;
		Cell* closestCell = filterAndProcessNearby(cell->getPosition(), m_nearbyCells, CellSettings::visualRadius, cell->getRadius(), nearbyCellCount);
		Plant* closestPlant = filterAndProcessNearby(cell->getPosition(), m_nearbyPlants, PlantSettings::visualRange, cell->getRadius(), nearbyPlantCount);

		// setting the information in the cell to be used for later
		cell->setClosestEntities(closestCell, closestPlant, nearbyCellCount, nearbyPlantCount);
	}
}


void Simulation::updateCells()
{
	for (Cell* cell : m_Cells)
	{
		cell->update();

		if (!cell->thermalToggle(m_thermal)) continue;

		bufferColorUpdate({ cell->indexes }, cell->getColor());
	}

	updateEntityPosition(m_Cells);


}


void Simulation::clearEntityData()
{
	// clearing the current simulation data
	for (Cell* cell : m_Cells)    cell->die();
	for (Plant* plant : m_Plants) plant->die();

	addAndRemoveEntities(m_Cells, true);
	addAndRemoveEntities(m_Plants, false);
}


template<class E, unsigned N>
void Simulation::updateEntityPosition(o_vector<E, N>& entities)
{
	for (E* entity : entities)
	{
		entity->updatePositioning();
		bufferPosUpdate({entity->indexes}, entity->getDeltaPos());
	}
}


template<class E, unsigned N>
void Simulation::addAndRemoveEntities(o_vector<E, N>& entities, const bool isCell) {
	for (E* entity : entities)
	{
		if (entity->isDead())
			removeEntity(entity, isCell);

		else if (entity->shouldReproduce())
			addEntity(entities, entity, isCell);
	}
}


void Simulation::overflowProtection(const unsigned maxcells, const unsigned maxplants)
{
	overflowCheckEntities(m_Cells, maxcells, true);
	overflowCheckEntities(m_Plants, maxplants, false);
}

void Simulation::extinctionCheck()
{
	// validating that we should proceed
	if (!autoExtinctionReset || m_Cells.size() > 0 || maxCells == 0 || initCellCount == 0)
		return;

	// correctly re-sizing all entities
	plantUnderflowProtection(initPlantCount);
	overflowCheckEntities(m_Plants, initPlantCount, false);

	for (unsigned i{ 0 }; i < initCellCount; i++)
	{
		Cell* cell = m_Cells.add();

		const sf::Vector2f chosenPosition = randPosInRect(m_simBounds);
		const sf::Vector2f deltaPosition = chosenPosition - cell->getPosition();
		cell->setEntityPosition(chosenPosition);

		bufferPosUpdate({ cell->indexes }, deltaPosition);
		bufferColorUpdate({ cell->indexes }, cell->getColor());
	}

	totalExtinctions++;
	relativeFrameCount = 0;
	std::cout << "extinction " << totalExtinctions << " occoured at frame " << totalFrameCount << "\n";
}

template<class E, unsigned N>
void Simulation::overflowCheckEntities(o_vector<E, N>& entities, const unsigned maxEntities, const bool type) {
	while (entities.size() > maxEntities)
	{
		for (E* entity : entities)
		{
			if (randint(0, 10) != 0) continue;
			removeEntity(entity, type);

			if (entities.size() <= maxEntities)
				break;
		}
	}
}

template<class E>
void Simulation::removeEntity(E* entity, const bool type)
{ // type : true = cell, type : false = plant
	if (type == true)
		m_Cells.remove(entity->vector_id);
	else
		m_Plants.remove(entity->vector_id);

	// the deathPos is where all the dead entities go to
	const sf::Vector2f deathPos = { -100.f, -100.f };
	const sf::Vector2f deltaPos = deathPos - entity->getPosition();
	entity->setEntityPosition(deathPos);
	bufferPosUpdate({ entity->indexes }, deltaPos);

	entity->wipeData();
}


template<class E, unsigned N>
bool Simulation::addEntity(o_vector<E, N>& entities, E* entity, const bool isCell)
{
	E* newEntity = entities.add();
	if (newEntity == nullptr)
		return false;

	entity->reproduce(newEntity);
	bufferPosUpdate({newEntity->indexes}, newEntity->getDeltaPos());

	sf::Color color = Plant::generateColor();
	if (isCell)
		color = newEntity->getColor();

	bufferColorUpdate({ newEntity->indexes }, color);

	return true;
}
