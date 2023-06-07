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
			m_closeSim = true;

		else if (event.type == sf::Event::KeyPressed)
			keyPressEvents(event.key.code);

		else if (event.type == sf::Event::MouseWheelScrolled)
			zoom(event.mouseWheelScroll.delta);

		else if (event.type == sf::Event::MouseButtonPressed)
			m_mousePressed = true;

		else if (event.type == sf::Event::MouseButtonReleased)
			m_mousePressed = false;
	}
}


void Simulation::updateCellStatistics()
{
	float offspringSum = 0;
	float ageSum = 0;

	for (const Cell* cell : m_Cells)
	{
		offspringSum += static_cast<float>(cell->offspringCount);
		ageSum += static_cast<float>(cell->getAge());
	}

	const auto size = static_cast<float>(m_Cells.size());
	avgLifeTime.push_back(ageSum / size);
	avgReproCount.push_back(offspringSum / size);
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

	case sf::Keyboard::Key::B:
		m_debugBorder = not m_debugBorder;
		break;

	case sf::Keyboard::Key::F:
		m_frameByFrame = not m_frameByFrame;
		m_paused = m_frameByFrame;
		
		break;

	case sf::Keyboard::Key::T:
		m_thermal = not m_thermal;

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

	case sf::Keyboard::Key::H:
		if (shifting)
			m_debugCircToggle = not m_debugCircToggle;
		break;

	case sf::Keyboard::Key::N:
		if (shifting)
			m_debugClosestToggle = not m_debugClosestToggle;
		break;

	case sf::Keyboard::Key::A:
		if (ctrl)
		{
			m_autoSaving = not m_autoSaving;
			std::cout << "AutoSaving: " << m_autoSaving;
		}
		break;

	case sf::Keyboard::Key::S:
		if (ctrl)
			saveData();
		break;

	case sf::Keyboard::Key::L:
		if (ctrl)
			loadData();
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
		m_window.draw(m_hashGrid.m_renderGrid, getStates());

	if (m_debugBorder)
		drawRectOutline(m_simBounds, m_window, getStates());

	displayFrameRate(m_window, "Cellular Simulation", m_clock);
	m_window.display();
}


void Simulation::debugEntities()
{
	for (const Plant* plant : m_Plants)
		debugEntity(plant, PlantSettings::visualRange, PlantSettings::initMass);

	for (const Cell* cell : m_Cells)
		debugEntity(cell, CellSettings::visualRadius, cell->getRadius());
}


void Simulation::debugEntity(const Entity* entity, const float vrange, const float initRad)
{
	const float rad = debugCircle.getRadius();
	const sf::Vector2f position = entity->getPosition();

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

	if (m_debugVelToggle)
	{
		// Normalize the velocity vector
		constexpr float normLength = 7.f;
		const sf::Vector2f velocity = normaliseVector(entity->getVelocity(), normLength);
		const sf::Vector2f displacement = normaliseVector(entity->getDisplacement(), normLength);

		m_window.draw(makeLine(position, position + velocity    , { 255, 0  , 255 }), getStates());
		m_window.draw(makeLine(position, position + displacement, { 0, 255, 100 }), getStates());
	}

	if (m_debugClosestToggle)
	{
		const sf::VertexArray velLine = makeLine(position, entity->getClosestPos(), { 0, 0, 255 });
		m_window.draw(velLine, getStates());
	}
}