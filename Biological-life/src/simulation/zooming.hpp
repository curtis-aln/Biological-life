#pragma once

#include <SFML/Graphics.hpp>

/*
A class dedacated to managing translation and zoom of a surface for SFML 
todo:
- get bordering to work
 */

class ZoomManagement
{
	sf::RenderStates m_renderStates{};
	sf::Vector2f m_mousePositionBefore{};

	float m_currentScroll = 1.f;

	const float m_zoomStrength = 0.22f;
	const float m_originalScroll{};

	const sf::Rect<float> m_boundery;
	sf::Vector2f m_currentTranslation = { m_boundery.left + m_boundery.width / 2, m_boundery.top + m_boundery.height / 2 };


public:
	explicit ZoomManagement(const sf::Rect<float>& boundery, const float scale)
	: m_originalScroll(m_currentScroll * scale), m_boundery(boundery)
	{
		m_currentScroll *= scale;
		m_renderStates.transform.scale({ scale, scale });
	}


	sf::Vector2f updateMousePos(const sf::Vector2f mousePos)
	{
		const sf::Vector2f deltaPos = mousePos - m_mousePositionBefore;
		m_mousePositionBefore = mousePos;
		return deltaPos;
	}


	sf::RenderStates& getStates() { return m_renderStates; }


	void update(const sf::Vector2f& offset, const float scale)
	{
		// apply the translation offset
		m_renderStates.transform.translate(offset);

		m_currentScroll *= scale;
		m_renderStates.transform.scale({ scale, scale });

		// apply the opposite of the translation offset
		m_renderStates.transform.translate(-offset);
	}


	void translate(const sf::Vector2f delta)
	{
		const sf::Vector2f finalTranslation = { delta.x / m_currentScroll, delta.y / m_currentScroll };
		rawTranslate(finalTranslation);
	}


	void zoom(const float deltaScroll)
	{
		const float scale = getZoomScale(deltaScroll);
		zoomWithScale(m_mousePositionBefore, scale);
	}


private:
	[[nodiscard]] float getZoomScale(const float delta) const
	{
		if (delta > 0) return 1.0f + m_zoomStrength;
		return 1.0f - m_zoomStrength;
	}

	void rawTranslate(const sf::Vector2f finalTranslation)
	{
		m_currentTranslation += finalTranslation;
		m_renderStates.transform.translate(finalTranslation);
	}

	void zoomWithScale(const sf::Vector2f& mousePos, float scale)
	{
		if (m_currentScroll * scale < m_originalScroll)
			scale = m_originalScroll / m_currentScroll;

		const auto offset = m_renderStates.transform.getInverse().transformPoint(mousePos);
		update(offset, scale);
	}
};