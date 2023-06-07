#pragma once

#include "SFML/Graphics.hpp"

#include "entity.hpp"
#include "plant.hpp"
#include "genome.hpp"
#include "../settings.hpp"

#include <nlohmann/json.hpp>

#include <boost/functional/hash.hpp>
#include <vector>




class EnergyManagement
{
private:
	static constexpr float K = 0.00054f;
	static constexpr float reproThresh = 100.f;
	float m_energy = initialEnergy;
	
protected:
	static constexpr float initialEnergy = 50.f;

	void updateEnergy(const sf::Vector2f velocity, const float mass, const unsigned age, const unsigned nearby)
	{
		const float speed = abs(velocity.x) + abs(velocity.y);
		const float a_age = static_cast<float>(age) * 0.01f;
		const float deltaEnergy = (mass + speed + a_age) * K;
		m_energy -= deltaEnergy;
	}

	[[nodiscard]] bool reproCheck(const float mass) const { return m_energy > reproThresh; }
	[[nodiscard]] float getEnergy() const { return m_energy; }
	[[nodiscard]] bool energyDeathCheck() const { return m_energy <= 0.f; }

	void setEnergy(const float newEnergy) { m_energy = newEnergy; }

	void energyDiffusion(EnergyManagement& otherSystem, const float diffusion)
	{
		const float delta = (otherSystem.m_energy - this->m_energy) * diffusion;
		this->m_energy += delta;
		otherSystem.m_energy -= delta;

		if (this->m_energy < 0)
		{
			otherSystem.m_energy -= std::abs(this->m_energy);
			this->m_energy = -0.1f;
			otherSystem.m_energy += 0.1f;
		}

		else if (otherSystem.m_energy < 0)
		{
			this->m_energy -= std::abs(otherSystem.m_energy);
			otherSystem.m_energy = -0.1f;
			this->m_energy += 0.1f;
		}
	}
};


class Cell : public Entity, public Genome, CellSettings, EnergyManagement, Perceptron
{
	unsigned m_timeAlone = 0;
	unsigned m_reproduceCounter = 0;

	Cell* m_closestCell = nullptr;
	Plant* m_closestPlant = nullptr;

	float m_maxSpeed{};
	float uniqueIdentifier{};


public:
	unsigned offspringCount = 0;
	unsigned vector_id = 0;

	// constructor and destructor
	explicit Cell(const Entity& entity = {}, const Genome& genome = Genome(), const unsigned Vector_id = 0)
	: Entity(entity), Genome(genome), Perceptron(sensoryInputs, numHiddenLayers, hiddenLayerSize, sensoryOutputs), vector_id(Vector_id)
	{
		setEntityRadius(m_entityRadius);
		uniqueIdentifier = generateUniqueIdentifier(getInputHiddenWeights());
	}

	Cell& operator=(const Cell& other)
	{
		if (this == &other)
			return *this;  // Check for self-assignment

		Entity::operator=(other);
		Perceptron::operator=(other);

		m_timeAlone        = other.m_timeAlone;
		m_reproduceCounter = other.m_reproduceCounter;
		m_closestCell      = other.m_closestCell;
		m_closestPlant     = other.m_closestPlant;
		m_maxSpeed         = other.m_maxSpeed;
		uniqueIdentifier   = other.uniqueIdentifier;
		offspringCount     = other.offspringCount;
		return *this;
	}

	void wipeData()
	{
		wipeEntityData();
		setEnergy(initialEnergy);
		offspringCount = 0;
		m_reproduceCounter = 0;
		m_timeAlone = 0;
	}


	bool thermalToggle(const bool thermalOn)
	{
		if (thermalOn)
		{
			const auto fit = static_cast<int>(getEnergy() * 2.f);
			const auto col = static_cast<sf::Uint8>(std::max(0, std::min(255, fit)));
			m_color = { col, col, col, 200 };
			return true;

		}

		if (thermalOn == false && m_color != m_originalColor)
		{
			m_color = m_originalColor;
			return true;
		}
		return false;
	}


	void setClosestEntities(Cell* closestCell, Plant* closestPlant, const unsigned nearbyCells, const unsigned nearbyPlants)
	{
		m_closestCell  = closestCell;
		m_closestPlant = closestPlant;
		m_nearbyCells  = nearbyCells;
		m_nearbyPlants = nearbyPlants;

		setClosestPos();
	}

	nlohmann::json saveCellJson()
	{
		return {
			{"entity data", saveEntityData()   },
			{"network data", saveNetworkJson() },
			{"time alone", m_timeAlone         },
			{"reproduce counter", m_reproduceCounter},
			{"radius", getRadius() },
			{"energy", getEnergy() }
		};
	}

	void loadCellData(const nlohmann::json& cellData)
	{
		loadEntityData(cellData["entity data"]);
		loadNetworkData(cellData["network data"]);
		m_timeAlone        = cellData["time alone"];
		m_reproduceCounter = cellData["reproduce counter"];
		m_entityRadius     = cellData["radius"];
		dead = false;

		setEnergy(cellData["energy"]);
	}


	void reproduce(Cell* cell)
	{
		// preparing this cell
		setEnergy(getEnergy() / 2);
		reporoduce = false;
		offspringCount++;

		constexpr float va = 3.f;
		const sf::Vector2f pos = m_positionCurrent + randVector(-va, va, -va, va);

		cell->m_velocity = m_velocity * -1.f;
		cell->updatePositionWithVelocity();
		cell->m_clippingDisplacement = (pos - cell->m_positionCurrent);
		cell->setEnergy(getEnergy());

		cell->m_color = createMutatedColor(cell->m_color);
		mutate(*cell);

		cell->updateDisplacement();
		cell->m_closestEntityPos = cell->m_positionCurrent;
		cell->uniqueIdentifier = generateUniqueIdentifier(getInputHiddenWeights());
	}


	void update()
	{
		cellInteraction();
		plantInteraction();

		speed_limit(m_maxSpeed);
		m_maxSpeed = weightedOutputs[2] * 10.f;
		applyFriction(1 + std::abs(weightedOutputs[4]));
		collisionManagement();

		// end of function statistics update
		updateEnergy(m_velocity, m_entityRadius, age, m_nearbyCells);
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

		applyUpriciple(m_closestEntityPos.x);
		applyUpriciple(m_closestEntityPos.y);
	}

	void collisionManagement()
	{
		if (validateEntityPtr(m_closestCell))
		{
			if (entityCollision(m_closestCell))
				energyDiffusion(*m_closestCell, weightedOutputs[3]);
		}


		if (validateEntityPtr(m_closestPlant))
		{

			if (entityCollision(m_closestPlant))
			{
				// transfer of nutrience upon contact
				m_closestPlant->energy -= energyTransferRate;
				setEnergy(getEnergy() + energyTransferRate);
			}
		}
	}

	void reproOrDeathCheck()
	{
		if (m_timeAlone > maxTimeAlone || energyDeathCheck())
			die();

		if (m_reproduceCounter < reproductionDelay)
			m_reproduceCounter++;
		
		if (reproCheck(m_entityRadius))
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

	static void applyUpriciple(float& value)
	{
		value += randfloat(-0.055f, 0.055f);
	}


	void cellInteraction()
	{
		// cell validation
		if (!validateEntityPtr(m_closestCell))    { m_timeAlone++; return; }
		const float Celldist = distSquared(m_closestEntityPos, getPosition());
		if (Celldist > (visualRadius - 7) * (visualRadius - 7))    { m_timeAlone++; return; }
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
			static_cast<float>(m_nearbyPlants) / 10.f,  // plant count
			std::abs(m_closestCell->uniqueIdentifier - uniqueIdentifier) // similarity relation
		};
	
		this->compute_output(inputs);

		m_velocity += cellDirection * weightedOutputs[0]; // interaction with cell
	}

};