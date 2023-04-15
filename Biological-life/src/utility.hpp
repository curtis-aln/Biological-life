#pragma once

inline float dot(const sf::Vector2f& v1, const sf::Vector2f& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

inline float length(const sf::Vector2f& v)
{
	return std::sqrt(dot(v, v));
}

inline float lengthSquared(const sf::Vector2f vec)
{
	return vec.x * vec.x + vec.y * vec.y;
}

inline sf::Vector2f normalize(const sf::Vector2f& v)
{
	const float len = length(v);
	return (len > 0.f) ? v / len : v;
}

// random
inline int randint(const int start, const int end) {
	return rand() % (end - start) + start;
}

inline float randfloat(const float start, const float end)
{
	return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (end - start)) + start;
}

inline sf::Vector2f randVector(const float start1, const float end1, const float start2, const float end2)
{
	return { randfloat(start1, end1), randfloat(start2, end2) };
}

inline sf::Vector2f randPosInRect(const sf::Rect<float> rect)
{
	return randVector(rect.left, rect.left + rect.width, rect.top, rect.top + rect.height);
}


inline sf::Color randColor(
	const float rMin = 0, const float rMax = 255, const float gMin = 0, const float gMax = 255, 
	const float bMin = 0, const float bMax = 255)
{
	return {
		static_cast<sf::Uint8>(randfloat(rMin, rMax)),
		static_cast<sf::Uint8>(randfloat(gMin, gMax)),
		static_cast<sf::Uint8>(randfloat(bMin, bMax))
	};
}


inline sf::Vector2u clipToGrid(const sf::Vector2u position, const sf::Vector2u tileSize)
{
	const sf::Vector2u index(position.x / tileSize.x, position.y / tileSize.y);
	return { index.x * tileSize.x, index.y * tileSize.y};
}


inline float distSquared(const sf::Vector2f positionA, const sf::Vector2f positionB)
{
	const sf::Vector2f delta = positionB - positionA;
	return delta.x * delta.x + delta.y * delta.y;
}

inline sf::Rect<float> resizeRect(const sf::Rect<float>& rect, const sf::Vector2f resize)
{
	return {
		rect.left + resize.x,
		rect.top + resize.y,
		rect.width - resize.x * 2.f,
		rect.height - resize.y * 2.f
	};
}

inline sf::Vector2f getMousePositionFloat(const sf::RenderWindow& window)
{
	const sf::Vector2f mousePosition = {
		static_cast<float>(sf::Mouse::getPosition(window).x),
		static_cast<float>(sf::Mouse::getPosition(window).y)
	};
	return mousePosition;
}


inline sf::Vector2f normaliseVector(sf::Vector2f vector, const float size)
{
	// Normalize the velocity vector
	if (const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y); length > 0)
		vector /= length;

	vector *= size;

	return vector;
}

inline void displayFrameRate(sf::RenderWindow& window, const std::string& title, sf::Clock& clock)
{
	// FPS management
	const sf::Int32 msPerFrame = clock.restart().asMilliseconds();

	std::ostringstream oss;
	oss << title << " " << msPerFrame << " ms/frame" << "\n";
	const std::string stringFrameRate = oss.str();
	window.setTitle(stringFrameRate);
}

inline double roundToNearestN(const double value, const unsigned int decimal_places) {
	const double multiplier = pow(10, decimal_places);
	return round(value * multiplier) / multiplier;
}

inline sf::VertexArray drawLine(const sf::Vector2f& point1, const sf::Vector2f& point2, const sf::Color color)
{
	sf::VertexArray line(sf::Lines, 2);
	line[0].position = point1;
	line[1].position = point2;
	line[0].color = color;
	line[1].color = color;
	return line;
}