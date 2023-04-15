#pragma once

#include "SFML/Graphics.hpp"

#include "entity.hpp"
#include "Plant.hpp"
#include "genome.hpp"

#include <nlohmann/json.hpp>


struct CellSettings
{
	inline static constexpr float frictionCoefficient = 1.4f;
	inline static constexpr float visualRange = 68;

	// rate at which m_energy is transered from plant to cell
	inline static constexpr float nutrienceRate = 2.f;

	inline static constexpr int maxTimeAlone = 30;
	inline static constexpr int reproductionDelay = 40;
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


class Cell : public Entity, public Genome, CellSettings, EnergyManagement
{
	unsigned int m_timeAlone = 0;
	unsigned int m_reproduceCounter = 0;

public:
	// constructor and destructor
	explicit Cell(const Entity& entity, const Genome& genome) : Entity(entity), Genome(genome)
	{
		// random initial velocity
		const float speed = getSpeedMax();
		m_velocity = randVector(-speed, speed, -speed, speed);

		setEntityRadius(getGenomeRadius());
	}

	void wipeData()
	{
		wipeEntityData();
		setEnergy(initialEnergy);
	}

	nlohmann::json saveCellJson()
	{
		return {
			{"entity data", saveEntityData()},
			{"genome data", saveGenomeData()},
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

		constexpr float va = 3.f;
		const sf::Vector2f pos = m_positionCurrent + randVector(-va, va, -va, va);

		cell->m_velocity = m_velocity * -1.f;
		cell->updatePosition();

		cell->m_clippingDisplacement = (pos - cell->m_positionCurrent);

		cell->setEnergy(getEnergy());
		createMutatedClone(*cell);

		cell->updateDisplacement();
	}



	void update(const float deltaTime, Cell* closestCell, Plant* closestPlant)
	{
		cellInteraction(closestCell);

		speed_limit(getSpeedMax());

		// Resistance = K * (Speed^2) / Radius
		const float speedSQ = m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y;
		const float resitance = frictionCoefficient * speedSQ / getRadius();
		applyFriction(1 + resitance);


		if (closestCell != nullptr && !closestCell->isDead())
		{
			if (entityCollision(closestCell))
				energyDiffusion(*closestCell);
			m_closestEntityPos = closestCell->getPosition();
		}
		else
		{
			m_closestEntityPos = m_positionCurrent;
		}

		if (closestPlant != nullptr && !closestPlant->isDead())
		{
			if (entityCollision(closestPlant))
			{
				// transfuring nutrience
				closestPlant->energy -= nutrienceRate;
				setEnergy(getEnergy() + nutrienceRate);
			}
		}

		updateEnergy(m_velocity, getRawMass(), age);

		reproOrDeathCheck();

		age++;
	}



	void updatePositioning()
	{
		updatePosition();
		border();
		updateDisplacement();
	}


private:
	void reproOrDeathCheck()
	{
		if (m_timeAlone > maxTimeAlone || energyDeathCheck())
			die();
		
		if (reproCheck(getGenomeRadius()))
		{
			if (++m_reproduceCounter >= reproductionDelay)
			{
				prepReproduction();
				m_reproduceCounter = 0;
			}
		}
	}


	void cellInteraction(const Cell* closestCell)
	{
		constexpr float minDist = (visualRange - 7) * (visualRange - 7);
		if (closestCell == nullptr || distSquared(closestCell->getPosition(), getPosition()) > minDist)
		{
			m_timeAlone++;
			return;
		}

		m_timeAlone = 0;
		const sf::Vector2f direction = closestCell->getPosition() - this->getPosition();
		m_velocity += direction * this->calculateInteration();
	}
};