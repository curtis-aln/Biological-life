#pragma once

#include "SFML/Graphics.hpp"

#include "entity.hpp"

/*
 * TODO:
 * in the future have different "Spicies" of Plant so that they only attract to plant that are similar in spicies,
 * have their similarity as a value that mutates over time so they become less and less similar causing them to be
 * repelled
 *
 */


struct PlantSettings
{
	inline static constexpr float initMass = 8.f;
	inline static constexpr float visualRange = initMass * 6.5f; // must be smaller than a grid cell

protected:
	inline static constexpr float growSpeed = 0.05f;

	inline static constexpr float reproMass = 10;
	inline static constexpr float reproAmount = 3;
	inline static constexpr unsigned reproAge = 10'000;
	inline static constexpr unsigned reproMinCollisions = 10;
	inline static constexpr unsigned randDeathChance = 1000;

	inline static constexpr float friction = 1.00f;
	inline static constexpr float maxSpeed = .2f;
	inline static constexpr float attractStrength = 0.0002f;

	inline static constexpr float plantEnergy = 50.f;
};


class Plant : public Entity, PlantSettings
{
	std::vector<Plant> nearby;
	c_Vec collisionIndexes;

	// unique spicies identifier
	float usi;


	void interactWithNearby(const std::vector<Plant*>& nearbyPlants)
	{
		collisionIndexes.size = 0;
		if (nearbyPlants.empty())
			return;

		m_closestEntityPos = m_positionCurrent;

		int incrementer = 0;
		for (const Plant* plant : nearbyPlants)
		{
			// making sure the plant is valid
			if (plant == nullptr || plant == this || distSquared(plant->getPosition(), getPosition()) > visualRange * visualRange)
			{ incrementer++; continue; }

			collisionIndexes.add(incrementer);

			float interaction = attractStrength;
			if (abs(this->usi - plant->usi) > 2.f)
				interaction = -0.1f;

			m_velocity += (plant->getPosition() - m_positionCurrent) * interaction;

			incrementer++;
		}

	}

public:
	float energy = plantEnergy;
	sf::Color color;


	// constructor and destructor
	explicit Plant(const Entity& entity, const float Usi) : Entity(entity), usi(Usi)
	{
		setEntityRadius(initMass);
	}

	static sf::Color generateColor()
	{
		sf::Color color = randColor(0, 100, 180, 255, 0, 100);
		color.a = 150;
		return color;
	}

	[[nodiscard]] sf::Color getColor() const {return color;}

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
			{"color", {"r", color.r}, {"g", color.g}, {"b", color.b}},
		};
	}


	void reproduce(Plant* plant)
	{
		// preparing this cell
		this->reporoduce = false;
		this->age = 0;

		constexpr float r = 10.f;
		const sf::Vector2f pos = { m_positionCurrent.x + randfloat(-r, r), m_positionCurrent.y + randfloat(-r, r) };

		plant->energy = plantEnergy;
		plant->m_clippingDisplacement = pos - plant->m_positionCurrent;
		plant->updateDisplacement();
		plant->usi = usi + randfloat(-2, 2);
	}

	void wipeData()
	{
		wipeEntityData();
		age = 0;
		energy = plantEnergy;
	}

	void createRandom()
	{
		energy = plantEnergy;
		const sf::Vector2f desiredPosition = randPosInRect(*m_border);
		m_clippingDisplacement += desiredPosition - m_positionCurrent;
		m_velocity = randVector(-2, 2, -2, 2);
		m_deltaPos = m_clippingDisplacement;
		m_positionCurrent = desiredPosition;
	}


	void processEntityCollisions(const std::vector<Plant*>& nearbyPlants)
	{
		for (unsigned int i{ 0 }; i < collisionIndexes.size; i++)
		{
			Plant* plant = nearbyPlants[collisionIndexes.at(i)];
			entityCollision(plant);
		}
	}

	void update(const std::vector<Plant*>& nearbyPlants)
	{
		interactWithNearby(nearbyPlants);
		moveToCenter();
		speed_limit(maxSpeed);
		applyFriction(friction);
		borderRepultion();
		updatePosition();

		processEntityCollisions(nearbyPlants);

		if (age > reproAge && collisionIndexes.size < reproMinCollisions)
			prepReproduction();

		if (energy <= 0 || randint(0, randDeathChance) == 0)
			die();

		age += randint(-10, 100);
	}
};