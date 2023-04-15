#include "Simulation.hpp"
#include "../Life/entity.hpp"

void Simulation::run()
{
	while (!m_closeSim)
	{
		runFrame(GetDelta());
		renderFrame();
	}
}


void Simulation::runFrame(const double deltaTime)
{
	if (!m_paused)
	{
		prepGrid();

		addAndRemoveEntities(m_allCells, true);
		addAndRemoveEntities(m_allPlants, false);

		updatePlants();
		updateCells();

		overflowProtection(maxCells, maxPlants); // bug: bottleneck
		plantUnderflowProtection(minPlants);
		extinctionCheck();

		if ((totalFrameCount % 1000) == 0)
			alignEntites(m_allCells);

		m_buffer.update();
	}

	// updating runtime statistics and ending the frame
	totalFrameCount++;
	relativeFrameCount++;

	float increase = 0.0000005f;

	if (m_allCells.size() > 2000)
		increase *= static_cast<float>(m_allCells.size()) / 1000.f;

	if (m_allCells.size() > 1300)
		min_speed += increase;

	totalRunTime += deltaTime;

	if (totalFrameCount % 1000 == 0)
		printStatistics();

	//if (totalFrameCount > 10000)
	//	m_closeSim = true;

}

template<class E>
void Simulation::alignEntites(o_vector<E>& entities)
{
	for (const unsigned int index : entities.getAvalableIndexes())
	{
		E& entity = entities.at(index);
		sf::Vector2f& actualCenter = m_buffer.getVertices()->at(entity.indexes[0] + 2).position;
		sf::Vector2f desiredCenter = entity.getPosition();
		sf::Vector2f delta = desiredCenter - actualCenter;

		bufferPosUpdate({ entity.indexes }, delta);
	}
}


void Simulation::prepGrid()
{
	grid.clear();

	// first loop is for adding the cells
	for (const unsigned int index : m_allCells.getAvalableIndexes())
		grid.addAtom(m_allCells.at(index).getPosition(), encodeEntityToId(static_cast<int>(index), true));

	// second loop is for adding the plants
	for (const unsigned int index : m_allPlants.getAvalableIndexes())
		grid.addAtom(m_allPlants.at(index).getPosition(), encodeEntityToId(static_cast<int>(index), false));

}


void Simulation::updatePlants()
{
	for (const unsigned int index : m_allPlants.getAvalableIndexes())
	{
		Plant& plant = m_allPlants.at(index);

		m_nearbyCells.clear();
		m_nearbyPlants.clear();

		decodeEntityIds(m_nearbyCells, m_nearbyPlants, grid.find(plant.getPosition()));

		plant.update(1, m_nearbyPlants);
	}

	positionAlignment(m_allPlants);
}

void Simulation::updateCells()
{
	// this iteration updates all of the positions of the cells
	for (const unsigned int index : m_allCells.getAvalableIndexes())
	{
		Cell& cell = m_allCells.at(index);

		m_nearbyCells.clear();
		m_nearbyPlants.clear();

		decodeEntityIds(m_nearbyCells, m_nearbyPlants, grid.find(cell.getPosition()));

		if (m_nearbyCells.size() >= c_Vec::max / 4)
			cell.die();

		Cell* closestCell = filterAndProcessNearby(cell.getPosition(), m_nearbyCells, CellSettings::visualRange, cell.getGenomeRadius());
		Plant* closestPlant = filterAndProcessNearby(cell.getPosition(), m_nearbyPlants , PlantSettings::visualRange, cell.getGenomeRadius());
		cell.update(1, closestCell, closestPlant);
	}

	positionAlignment(m_allCells);
}

template<class E>
void Simulation::positionAlignment(o_vector<E>& entities)
{
	for (const unsigned int index : entities.getAvalableIndexes())
	{
		E& entity = entities.at(index);

		entity.updatePositioning();
		bufferPosUpdate({entity.indexes}, entity.getDeltaPos());
	}
}


template<class E>
void Simulation::addAndRemoveEntities(o_vector<E>& entities, const bool isCell) {
	for (const unsigned int index : entities.getAvalableIndexes())
	{
		E& entity = entities.at(index);

		bool otherDeath = false;
		if (isCell)
		{
			const sf::Vector2f vel = entity.getVelocity();
			const float speedSq = vel.x * vel.x + vel.y * vel.y;

			otherDeath = (speedSq < min_speed * min_speed) && (totalFrameCount % 50) == 0;
		}
		
		if (entity.isDead() || otherDeath)
		{
			entity.wipeData();
			removeEntity(entity, index, isCell);
		}

		else if (entity.shouldReproduce())
			addEntity(entities, entity, isCell);
	}
}


void Simulation::removeEntity(Entity& entity, const unsigned relativeIndex, const bool type)
{ // type : true = cell, type : false = plant
	if (type == true)
		m_allCells.remove(relativeIndex);
	else
		m_allPlants.remove(relativeIndex);

	
	const sf::Vector2f deathPos = { -100.f, -100.f };
	const sf::Vector2f deltaPos = deathPos - entity.getPosition();
	entity.setPosition(deathPos);
	bufferPosUpdate({ entity.indexes }, deltaPos);
}


template<class E>
bool Simulation::addEntity(o_vector<E>& entities, E& entity, const bool isCell)
{
	E* newEntity = entities.add();
	if (newEntity == nullptr)
		return false;

	entity.reproduce(newEntity);
	bufferPosUpdate({newEntity->indexes}, newEntity->getDeltaPos());

	sf::Color color = Plant::generateColor();
	if (isCell)
		color = newEntity->getColor();

	bufferColorUpdate({ newEntity->indexes }, color);

	return true;
}
