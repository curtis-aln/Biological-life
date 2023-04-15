#include "Simulation.hpp"

#include <SFML/Graphics.hpp>
#include "../utility.hpp"

void Simulation::pollEvents()
{
	const sf::Vector2f delta = updateMousePos(getMousePositionFloat(m_window));

	if (m_mousePressed)
		translate(delta);

	sf::Event event{};
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_closeSim = true;
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			keyPressEvents(event.key.code);
		}

		else if (event.type == sf::Event::MouseWheelScrolled)
		{
			zoom(beforeMousePos, event.mouseWheelScroll.delta);
		}

		else if (event.type == sf::Event::MouseButtonPressed)
		{
			m_mousePressed = true;
		}

		else if (event.type == sf::Event::MouseButtonReleased)
		{
			m_mousePressed = false;
		}
	}
}


void Simulation::printStatistics()
{
	std::cout << "-------------------------------------------------- " << ++updateCounter << "\n";
	std::cout << "Min Speed   : " << roundToNearestN(min_speed, 3) << "\n";
	std::cout << "Total Alive : " << m_allCells.size() << " Cells, " << m_allPlants.size() << " Plants" << "\n";
	std::cout << "Total Frames: " << totalFrameCount << "\n";
	std::cout << "Rel Frames  : " << relativeFrameCount << "\n";
	std::cout << "Extinctions : " << totalExtinctions << "\n";
	std::cout << "Time Passed : " << roundToNearestN(totalRunTime, 1) << " seconds \n";
	std::cout << "            : " << roundToNearestN(totalRunTime / 60, 4) << " mins \n";
	std::cout << "            : " << roundToNearestN((totalRunTime / 60) / 60, 4) << " hours \n";
	std::cout << "\n";
}


void Simulation::keyPressEvents(const sf::Keyboard::Key& event_key_code)
{
	const bool shifting = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
	const bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);

	switch (event_key_code)
	{
	case sf::Keyboard::Escape:
		m_closeSim = true;
		break;

	case sf::Keyboard::Space:
		m_paused = not m_paused;
		break;

	case sf::Keyboard::Key::G:
		m_drawGrid = not m_drawGrid;
		break;

	case sf::Keyboard::Key::V:
		if (shifting)
			m_debugVRangeToggle = not m_debugVRangeToggle;
		break;

	case sf::Keyboard::Key::D:
		if (shifting)
			m_debugVelToggle = not m_debugVelToggle;
		break;

	case sf::Keyboard::Key::C:
		if (shifting)
			m_debugCenterToggle = not m_debugCenterToggle;
		break;

	case sf::Keyboard::Key::B:
		if (shifting)
			m_debugCircToggle = not m_debugCircToggle;
		break;

	case sf::Keyboard::Key::Z:
		if (shifting)
			m_debugZoneToggle = not m_debugZoneToggle;
		break;

	case sf::Keyboard::Key::N:
		if (shifting)
			m_debugClosestToggle = not m_debugClosestToggle;
		break;

	case sf::Keyboard::Key::A:
		if (ctrl)
			m_autoSaving = not m_autoSaving;
		break;

	case sf::Keyboard::Key::S:
		if (ctrl)
			saveData();
		break;

	default:
		break;

	}
}


void Simulation::renderFrame()
{
	m_window.clear(windowColor);

	pollEvents();

	m_window.draw(*m_buffer.getBuffer(), getStates());

	debugEntities();

	// drawing grid
	if (m_drawGrid)
		m_window.draw(grid.m_renderGrid, getStates());

	displayFrameRate(m_window, "Cellular Simulation", m_clock);
	m_window.display();
}


void Simulation::debugEntities()
{
	for (const unsigned int index : m_allPlants.getAvalableIndexes())
	{
		Plant& plant = m_allPlants.at(index);
		debugEntity(plant, PlantSettings::visualRange, PlantSettings::initMass);
	}

	for (const unsigned int index : m_allCells.getAvalableIndexes())
	{
		Cell& cell = m_allCells.at(index);
		debugEntity(cell, CellSettings::visualRange, cell.getGenomeRadius());
	}
}


void Simulation::debugEntity(Entity& entity, const float vrange, const float initRad)
{
	const float rad = debugCircle.getRadius();
	const sf::Vector2f position = entity.getPosition();

	if (m_debugCenterToggle)
	{
		debugCircle.setPosition(position - sf::Vector2f{ rad, rad });
		m_window.draw(debugCircle, getStates());
	}

	if (m_debugVRangeToggle)
	{
		debugVRange.setPosition(position - sf::Vector2f{ vrange, vrange });
		debugVRange.setRadius(vrange);
		m_window.draw(debugVRange, getStates());
	}

	if (m_debugCircToggle)
	{
		debugCircleSize.setPosition(position - sf::Vector2f{ initRad, initRad });
		debugCircleSize.setRadius(initRad);
		m_window.draw(debugCircleSize, getStates());
	}

	if (m_debugZoneToggle)
		m_window.draw(debugSimZone, getStates());

	if (m_debugVelToggle)
	{
		// Normalize the velocity vector
		const sf::Vector2f velocity = normaliseVector(entity.getVelocity(), 5);
		const sf::Vector2f displacement = normaliseVector(entity.getDisplacement(), 5);

		// Draw the line with the new velocity
		const sf::VertexArray velLine = drawLine(position, position + velocity, { 0, 0, 255 });
		m_window.draw(velLine, getStates());

		const sf::VertexArray dispLine = drawLine(position, position + displacement, { 0, 255, 100 });
		m_window.draw(dispLine, getStates());
	}

	if (m_debugClosestToggle)
	{
		const sf::VertexArray velLine = drawLine(position, entity.getClosestPos(), { 0, 0, 255 });
		m_window.draw(velLine, getStates());
	}
}