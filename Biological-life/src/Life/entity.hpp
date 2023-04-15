#pragma once

#include "../buffer/Buffer.hpp"
#include "SFML/Graphics.hpp"
#include "../utility.hpp"

#include <cmath>
#include <nlohmann/json.hpp>


class Entity : public Allocations
{

protected:
	sf::Vector2f m_positionBefore;
	sf::Vector2f m_positionCurrent;

	sf::Vector2f m_closestEntityPos{};

	sf::Vector2f m_velocity;
	sf::Vector2f m_clippingDisplacement;
	sf::Vector2f m_deltaPos;

	const sf::Rect<float> m_border;

	float m_entityRadius;

	bool dead = false;
	bool reporoduce = false;


public:
	Entity(const Allocations& object, const sf::Rect<float> border)
	: Allocations(object), m_border(border)
	{
		
	}

	[[nodiscard]] sf::Vector2f getPosition() const { return m_positionCurrent; }
	[[nodiscard]] sf::Vector2f getClosestPos() const { return m_closestEntityPos; }
	[[nodiscard]] sf::Vector2f getVelocity() const { return m_positionCurrent - m_positionBefore; }
	[[nodiscard]] sf::Vector2f  getDeltaPos() const { return m_deltaPos; }
	[[nodiscard]] sf::Vector2f  getDisplacement() const { return m_clippingDisplacement; }
	[[nodiscard]] bool isDead() const { return dead; }
	[[nodiscard]] bool shouldReproduce() const { return reporoduce; }

	void setPosition(const sf::Vector2f newPosition)
	{
		m_positionBefore = newPosition;
		m_positionCurrent = newPosition;
		m_closestEntityPos = newPosition;
	}

	void setEntityRadius(const float radius) { m_entityRadius = radius; }
	float getRadius() const { return m_entityRadius; }


	nlohmann::json saveEntityData()
	{
		return {
			{"position before", {"x", m_positionBefore.x, "y", m_positionBefore.y}},
			{"position current", {"x", m_positionCurrent.x, "y", m_positionCurrent.y}},
			{"velocity", {"x", m_velocity.x, "y", m_velocity.y}},
		};
	}

	void die() { dead = true; }

protected:
	unsigned int age = 0;

	void wipeEntityData()
	{
		age = 0;
		dead = false;
		reporoduce = false;
		m_velocity = { 0, 0 };
		m_clippingDisplacement = { 0, 0 };
	}

	void prepReproduction() { reporoduce = true; }

	void updatePosition()
	{
		m_positionBefore = m_positionCurrent;
		m_positionCurrent += m_velocity;
	}

	void updateDisplacement()
	{
		const sf::Vector2f originalDisp = m_clippingDisplacement;
		m_positionCurrent += originalDisp;
		border();
		const sf::Vector2f deltaDisp = m_clippingDisplacement - originalDisp;
		m_positionCurrent += deltaDisp;


		m_deltaPos = m_velocity + originalDisp + deltaDisp;
		m_clippingDisplacement = { 0, 0 };
	}


	void applyFriction(const float strength)
	{
		m_velocity /= strength;
	}


	bool entityCollision(Entity* entity)
	{
		const float thisRad = getRadius();
		const float otherRad = entity->getRadius();

		const sf::Vector2f relative_position = entity->m_positionCurrent - m_positionCurrent;
		const float dist_squared = distSquared(this->m_positionCurrent, entity->m_positionCurrent);
		const float sum_radii = thisRad + otherRad;

		if (dist_squared >= sum_radii * sum_radii || dist_squared <= 0)
			return false;

		const float dist = std::sqrt(dist_squared);
		const sf::Vector2f normal_vector = relative_position / dist;
		const sf::Vector2f correction = (sum_radii - dist) * normal_vector * 0.5f;

		// Move the entities to prevent them from interpenetrating
		m_clippingDisplacement -= correction * (thisRad / sum_radii);
		entity->m_clippingDisplacement += correction * (otherRad / sum_radii);
		return true;
	}



	void border()
	{
		const float radius = getRadius();
		const sf::Vector2f desiredPos = {
			std::max(m_border.left + radius, std::min(m_positionCurrent.x, m_border.left + m_border.width - radius)),
			std::max(m_border.top + radius, std::min(m_positionCurrent.y, m_border.top + m_border.height - radius))
		};
		m_clippingDisplacement += desiredPos - m_positionCurrent;
	}


	void borderRepultion()
	{
		const float radius = getRadius();
		constexpr float buffer = 30.f;
		constexpr float repel = 0.02f;
		if (m_border.left + radius + buffer > m_positionCurrent.x)
			m_velocity.x += repel;

		else if (m_border.left + m_border.width - (radius + buffer) < m_positionCurrent.x)
			m_velocity.x -= repel;

		if (m_border.top + radius + buffer > m_positionCurrent.y)
			m_velocity.y += repel;

		else if (m_border.top + m_border.height - (radius + buffer) < m_positionCurrent.y)
			m_velocity.y -= repel;
	}


	void speed_limit(const float maxSpeed)
	{
		const float speedSQ = m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y;

		if (speedSQ > maxSpeed * maxSpeed)
		{
			const float speed = sqrt(speedSQ);
			m_velocity.x = (m_velocity.x / speed) * maxSpeed;
			m_velocity.y = (m_velocity.y / speed) * maxSpeed;
		}
	}
};



template <class T>
T* filterAndProcessNearby(const sf::Vector2f position, std::vector<T*>& entities, const float visualRange, const float radius)
{
	/* in this function we will find the closest cell to this current cell, while doing that we will also preform collision detection
	 */

	T* closestEntity = nullptr;
	float closestDistSq = visualRange * visualRange;

	for (T* otherEntity : entities)
	{
		const sf::Vector2f otherPos = otherEntity->getPosition();
		if (position == otherPos)
			continue;

		const float localDiam = radius + otherEntity->getRadius();
		const float distSq = lengthSquared(otherPos - position) - (localDiam * localDiam);

		if (distSq < closestDistSq)
		{
			closestEntity = otherEntity;
			closestDistSq = distSq;
		}
	}

	return closestEntity;
}