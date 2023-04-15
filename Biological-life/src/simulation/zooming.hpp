#pragma once

#include <SFML/Graphics.hpp>


struct ZoomManagement
{
	sf::RenderStates renderStates{};
	sf::Vector2f beforeMousePos{};

	const float m_zoomStrength = 0.22f;
	float scrollTracking = 1.f;

	const sf::Rect<float> zoomingBoundery;

	float initScale{};

	sf::Vector2f translationTracking = { zoomingBoundery.left + zoomingBoundery.width / 2, zoomingBoundery.top + zoomingBoundery.height / 2 };


	explicit ZoomManagement(const sf::Rect<float>& zoomingboundery, const float scale) : zoomingBoundery(zoomingboundery)
	{
		initScale = scrollTracking * scale;

		scrollTracking *= scale;
		renderStates.transform.scale({ scale, scale });
	}


	sf::Vector2f updateMousePos(const sf::Vector2f mousePos)
	{
		const sf::Vector2f deltaPos = mousePos - beforeMousePos;
		beforeMousePos = mousePos;
		return deltaPos;
	}

	[[nodiscard]] float getScale(const float delta) const
	{
		if (delta > 0)
			return 1.0f + m_zoomStrength;
		return 1.0f - m_zoomStrength;
	}


	sf::RenderStates& getStates() { return renderStates; }


	void update(const sf::Vector2f& offset, const float scale)
	{
		// apply the translation offset
		renderStates.transform.translate(offset);

		scrollTracking *= scale;
		renderStates.transform.scale({ scale, scale });

		// apply the opposite of the translation offset
		renderStates.transform.translate(-offset);
	}

	void translate(const sf::Vector2f delta)
	{
		const sf::Vector2f finalTranslation = { delta.x / scrollTracking, delta.y / scrollTracking };
		rawTranslate(finalTranslation);
	}

	void rawTranslate(const sf::Vector2f finalTranslation)
	{
		translationTracking += finalTranslation;
		renderStates.transform.translate(finalTranslation);
	}

	void zoom(const sf::Vector2f& mousePos, const float mouseWheelScroll_delta)
	{
		// bordering zoom
		const float scale = getScale(mouseWheelScroll_delta);
		zoomWithScale(mousePos, scale);
	}

	void zoomWithScale(const sf::Vector2f& mousePos, float scale)
	{
		if (scrollTracking * scale < initScale)
			scale = initScale / scrollTracking;

		const auto offset = renderStates.transform.getInverse().transformPoint(mousePos);
		update(offset, scale);
	}
};