#pragma once

#include "SFML/Graphics.hpp"

#include "entity.hpp"
#include "../settings.hpp"


class Plant : public Entity, PlantSettings
{
public:
	float energy = initialPlantEnergy;
	unsigned vector_id = 0;

private:
	std::vector<Plant> nearby;
	c_Vec m_collisionIndexes;

	float usi; // unique spicies identifier


	void interactWithNearby(const std::vector<Plant*>& nearbyPlants)
	{
		m_collisionIndexes.size = 0;
		if (nearbyPlants.empty())
			return;

		m_closestEntityPos = m_positionCurrent;

		int incrementer = 0;
		for (const Plant* plant : nearbyPlants)
		{
			// making sure the plant is valid
			if (plant == nullptr || plant == this || distSquared(plant->getPosition(), getPosition()) > visualRange * visualRange)
			{ incrementer++; continue; }

			m_collisionIndexes.add(incrementer);

			float interaction = attractStrength;
			if (abs(this->usi - plant->usi) > 2.f)
				interaction = -0.1f;

			m_velocity += (plant->getPosition() - m_positionCurrent) * interaction;

			incrementer++;
		}

	}

public:
	// constructor and destructor
	explicit Plant(const Entity& entity = {}, const float Usi = 0, const unsigned Vector_id = 0) : Entity(entity), usi(Usi), vector_id(Vector_id)
	{
		setEntityRadius(initMass);
	}

	Plant& operator=(const Plant& other)
	{
		if (this == &other)
			return *this;  // Check for self-assignment

		Entity::operator=(other);

		energy = other.energy;
		nearby = other.nearby;
		m_collisionIndexes = other.m_collisionIndexes;
		usi = other.usi;

		return *this;
	}

	static sf::Color generateColor()
	{
		sf::Color color = randColor(0, 100, 180, 255, 0, 100);
		color.a = 150; // transparancy
		return color;
	}

	void updatePositioning() { updateDisplacement(); }


	void moveToCenter()
	{
		const sf::Vector2f center = { m_border->left + m_border->width / 2, m_border->top + m_border->height / 2 };
		m_velocity += (center - m_positionCurrent) * 0.01f;
	}


	nlohmann::json savePlantJson()
	{
		return {
			{"entity data", saveEntityData()},
			{"radius", getRadius()},
			{"usi", usi},
			{"m_energy", energy},
		};
	}


	void reproduce(Plant* plant)
	{
		// preparing this cell
		this->reporoduce = false;
		this->age = 0;

		constexpr float va = 10.f;
		const sf::Vector2f pos = m_positionCurrent + randVector(-va, va, -va, va);

		plant->dead = false;
		plant->m_clippingDisplacement = pos - plant->m_positionCurrent;
		plant->updateDisplacement();
		plant->usi = usi + randfloat(-2, 2);
	}

	void wipeData()
	{
		wipeEntityData();
		age = 0;
		usi = 0;
		m_collisionIndexes.size = 0;
		energy = initialPlantEnergy;
	}

	void createRandom()
	{
		// setting the position of the plant
		const sf::Vector2f desiredPosition = randPosInRect(*m_border);

		dead = false;
		m_velocity = { 0.f, 0.f };
		updatePositionWithVelocity();
		m_clippingDisplacement = desiredPosition - m_positionCurrent;
		updateDisplacement();
	}


	void processEntityCollisions(const std::vector<Plant*>& nearbyPlants)
	{
		for (unsigned i{ 0 }; i < m_collisionIndexes.size; i++)
		{
			Plant* plant = nearbyPlants[m_collisionIndexes.at(i)];
			entityCollision(plant);
		}

		// todo
		if (m_collisionIndexes.size > 5)
			die();
	}

	void update(const std::vector<Plant*>& nearbyPlants)
	{
		interactWithNearby(nearbyPlants);
		moveToCenter();
		speed_limit(maxSpeed);
		applyFriction(friction);
		borderRepultion();
		updatePositionWithVelocity();

		processEntityCollisions(nearbyPlants);

		if (age > reproAge && m_collisionIndexes.size < reproMinCollisions)
			prepReproduction();

		if (energy <= 0 || randint(0, randDeathChance) == 0)
			die();

		age += randint(-10, 100);
	}
};