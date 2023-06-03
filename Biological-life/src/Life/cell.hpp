#pragma once

#include "SFML/Graphics.hpp"

#include "entity.hpp"
#include "plant.hpp"
#include "genome.hpp"

#include <nlohmann/json.hpp>


struct CellSettings
{
	inline static constexpr float visualRange = 68;

	// rate at which m_energy is transered from plant to cell
	inline static constexpr float nutrienceRate = 5.f;

	inline static constexpr int maxTimeAlone = 60;
	inline static constexpr int reproductionDelay = 40;

	inline static constexpr int sensoryInputs = 7;
	inline static constexpr int sensoryOutputs = 3;
};


class EnergyManagement
{
private:
	static constexpr float K = 0.00067f;
	static constexpr float reproThresh = 100.f;
	static constexpr float diffusion = 0.01f;
	float m_energy = initialEnergy;
	
protected:
	static constexpr float initialEnergy = 50.f;

	void updateEnergy(const sf::Vector2f velocity, const float mass, const unsigned int age)
	{
		const float speed = abs(velocity.x) + abs(velocity.y);
		const float deltaEnergy = (mass + speed + (static_cast<float>(age) * 0.02f)) * K;
		m_energy -= deltaEnergy;
	}

	[[nodiscard]] bool reproCheck(const float mass) const { return m_energy > reproThresh; }
	[[nodiscard]] float getEnergy() const { return m_energy; }
	[[nodiscard]] bool energyDeathCheck() const { return m_energy <= 0.f; }

	void setEnergy(const float newEnergy) { m_energy = newEnergy; }

	void energyDiffusion(EnergyManagement& otherSystem)
	{
		const float delta = (otherSystem.m_energy - this->m_energy) * diffusion;
		this->m_energy += delta;
		otherSystem.m_energy -= delta;
	}
};


class Cell : public Entity, public Genome, CellSettings, EnergyManagement, Perceptron
{
	unsigned int m_timeAlone = 0;
	unsigned int m_reproduceCounter = 0;
	unsigned int m_offspringCount = 0;

	Cell* m_closestCell = nullptr;
	Plant* m_closestPlant = nullptr;


public:
	// constructor and destructor
	explicit Cell(const Entity& entity, const Genome& genome) : Entity(entity), Genome(genome), Perceptron(sensoryInputs, sensoryOutputs)
	{
		setEntityRadius(getGenomeRadius());
	}

	void wipeData()
	{
		wipeEntityData();
		setEnergy(initialEnergy);
	}

	void setClosestEntities(Cell* closestCell, Plant* closestPlant, const unsigned nearbyCells, const unsigned nearbyPlants)
	{
		m_closestCell = closestCell;
		m_closestPlant = closestPlant;
		m_nearbyCells = nearbyCells;
		m_nearbyPlants = nearbyPlants;

		setClosestPos();
	}

	nlohmann::json saveCellJson()
	{
		return {
			{"entity data", saveEntityData()},
			{"genome data", saveGenomeData()},
			{"network data", saveNetworkJson()},
			{"time alone", m_timeAlone},
			{"reproduce counter", m_reproduceCounter},
			{"radius", getRadius()},
			{"m_energy", getEnergy()}
		};
	}


	void reproduce(Cell* cell)
	{
		// preparing this cell
		setEnergy(getEnergy() / 2);
		reporoduce = false;
		m_offspringCount++;

		constexpr float va = 3.f;
		const sf::Vector2f pos = m_positionCurrent + randVector(-va, va, -va, va);

		cell->m_velocity = m_velocity * -1.f;
		cell->updatePositionWithVelocity();
		cell->m_clippingDisplacement = (pos - cell->m_positionCurrent);
		cell->setEnergy(getEnergy());

		createMutatedClone(*cell);
		mutate(*cell);

		cell->updateDisplacement();
	}


	void update()
	{
		cellInteraction();
		plantInteraction();

		speed_limit(getSpeedMax());
		applyFriction(1 + std::abs(weightedOutputs[2]));
		collisionManagement();

		// end of function statistics update
		updateEnergy(m_velocity, getRawMass(), age);
		reproOrDeathCheck();
		age++;
	}



	void updatePositioning()
	{
		updatePositionWithVelocity();
		border();
		updateDisplacement();
	}


private:
	void setClosestPos()
	{
		if (validateEntityPtr(m_closestCell))
			m_closestEntityPos = m_closestCell->getPosition();
		else
			m_closestEntityPos = m_positionCurrent;
	}

	void collisionManagement()
	{
		if (validateEntityPtr(m_closestCell))
		{
			if (entityCollision(m_closestCell))
				energyDiffusion(*m_closestCell);
		}


		if (validateEntityPtr(m_closestPlant))
		{

			if (entityCollision(m_closestPlant))
			{
				// transfer of nutrience upon contact
				m_closestPlant->energy -= nutrienceRate;
				setEnergy(getEnergy() + nutrienceRate);
			}
		}
	}

	void reproOrDeathCheck()
	{
		if (m_timeAlone > maxTimeAlone || energyDeathCheck())
			die();

		if (m_reproduceCounter < reproductionDelay)
			m_reproduceCounter++;
		
		if (reproCheck(getGenomeRadius()))
		{
			if (m_reproduceCounter >= reproductionDelay)
			{
				prepReproduction();
				m_reproduceCounter = 0;
			}
		}
	}

	void plantInteraction()
	{
		if (!validateEntityPtr(m_closestPlant))
			return;

		const sf::Vector2f direction = m_closestPlant->getPosition() - this->getPosition();
		m_velocity += direction * weightedOutputs[1]; // interaction with plant
	}


	void cellInteraction()
	{
		// cell validation
		if (!validateEntityPtr(m_closestCell))    { m_timeAlone++; return; }
		const float Celldist = distSquared(m_closestEntityPos, getPosition());
		if (Celldist > (visualRange - 7) * (visualRange - 7))    { m_timeAlone++; return; }
		m_timeAlone = 0;

		// calculations for the cell
		const sf::Vector2f cellDirection = m_closestEntityPos - this->getPosition();
		const sf::Vector2f relCellDir = m_velocity - m_closestCell->getVelocity(); // relative velocity vector direction
		const float relativeCellSpeedSQ = relCellDir.x * relCellDir.x + relCellDir.y * relCellDir.y;

		// calculations for the plant
		float relPlantSpeed = 0; // by defualt 0
		float PlantDist = 0;     //
		if (validateEntityPtr(m_closestPlant))
		{
			const sf::Vector2f relPlantDir = m_velocity - m_closestPlant->getVelocity(); // relative velocity vector direction
			relPlantSpeed = relPlantDir.x * relPlantDir.x + relPlantDir.y * relPlantDir.y; // relative speed
			PlantDist = distSquared(m_closestPlant->getPosition(), getPosition()); // relative plant distance
		}

		// todo: optimize
		// calculating the computational output
		const std::vector inputs = {
			Celldist             / 4000.f, // cell distance SQ
			relativeCellSpeedSQ  / 5.f,   // direction to closest cell SQ
			PlantDist            / 4000.f, // plant distance SQ
			relPlantSpeed        / 5.f,   // direction to closest plant SQ
			getEnergy()          / 100.f,  // energy levels
			static_cast<float>(m_nearbyCells)  / 10.f, // cell count
			static_cast<float>(m_nearbyPlants) / 10.f  // plant count
		};

		//std::cout << formatVariables({
		//	{"cell distance", inputs[0]},
		//	{"rel cell speed", inputs[1]},
		//	{"plant distance", inputs[2]},
		//	{"rel plant speed", inputs[3]},
		//	{"energy", inputs[4]},
		//	{"nearby cells", inputs[5]},
		//	{"nearby plants", inputs[6]},
		//}) << "\n";
	
		this->compute_output(inputs);

		m_velocity += cellDirection * weightedOutputs[0]; // interaction with cell
	}

};