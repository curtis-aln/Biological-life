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

		addAndRemoveEntities(m_Cells, true);
		addAndRemoveEntities(m_Plants, false);

		updatePlants();
		updateCells();

		overflowProtection(maxCells, maxPlants); // bug: bottleneck
		plantUnderflowProtection(minPlants);
		extinctionCheck();

		if ((totalFrameCount % 1000) == 0)
			alignEntites(m_Cells);

		m_buffer.update();

		endFrame(deltaTime);
	}
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
	if (m_simBounds.left - m_DesiredBounds.left > 1)
		m_simBounds = resizeRect(m_simBounds, { -0.014f, -0.014f });

	// printing simulation statistics every 1000 frames
	if (totalFrameCount % 1000 == 0 && !m_paused)
		printStatistics();

	

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
	m_hashGrid.clear();

	// first loop is for adding the cells
	for (const unsigned int index : m_Cells.getAvalableIndexes())
		m_hashGrid.addAtom(m_Cells.at(index).getPosition(), encodeEntityToId(static_cast<int>(index), true));

	// second loop is for adding the plants
	for (const unsigned int index : m_Plants.getAvalableIndexes())
		m_hashGrid.addAtom(m_Plants.at(index).getPosition(), encodeEntityToId(static_cast<int>(index), false));

}


void Simulation::updatePlants()
{
	for (const unsigned int index : m_Plants.getAvalableIndexes())
	{
		Plant& plant = m_Plants.at(index);

		m_nearbyCells.clear();
		m_nearbyPlants.clear();

		decodeEntityIds(m_nearbyCells, m_nearbyPlants, m_hashGrid.find(plant.getPosition()));

		plant.update(m_nearbyPlants);
	}

	positionAlignment(m_Plants);
}

void Simulation::updateCells()
{
	// this iteration updates all of the positions of the cells
	for (const unsigned int index : m_Cells.getAvalableIndexes())
	{
		Cell& cell = m_Cells.at(index);

		m_nearbyCells.clear();
		m_nearbyPlants.clear();

		decodeEntityIds(m_nearbyCells, m_nearbyPlants, m_hashGrid.find(cell.getPosition()));

		// todo: experiment with this
		if (m_nearbyCells.size() >= c_Vec::max / 4)
			cell.die();

		unsigned nearbyCellCount = 0;
		unsigned nearbyPlantCount = 0;

		Cell* closestCell = filterAndProcessNearby(cell.getPosition(), m_nearbyCells, CellSettings::visualRange, cell.getGenomeRadius(), nearbyCellCount);
		Plant* closestPlant = filterAndProcessNearby(cell.getPosition(), m_nearbyPlants , PlantSettings::visualRange, cell.getGenomeRadius(), nearbyPlantCount);
		cell.setClosestEntities(closestCell, closestPlant, nearbyCellCount, nearbyPlantCount);
	}

	for (const unsigned int index : m_Cells.getAvalableIndexes())
	{
		m_Cells.at(index).update();
	}

	positionAlignment(m_Cells);
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
		if (isCell == true)
		{
			const sf::Vector2f vel = entity.getVelocity();
			const float speedSq = vel.x * vel.x + vel.y * vel.y;

			//bool cond1 = entity.m_nearbyCells > 2 && randint(0, 200) == 69;// || entity.m_nearbyCells < 2; //(speedSq < min_speed * min_speed) && (totalFrameCount % 20) == 0;
			//bool cond2 = false;// (entity.m_nearbyCells + entity.m_nearbyPlants < min_nearby) && (randint(0, 100) == 5);

			//otherDeath = speedSq > maxSpeed * maxSpeed && randint(0, 15) == 3;
			//if (cond1 || cond2) otherDeath = true;
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
		m_Cells.remove(relativeIndex);
	else
		m_Plants.remove(relativeIndex);

	
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
