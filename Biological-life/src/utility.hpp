#pragma once

#include <nlohmann/json.hpp>
#include <fstream> // for std::ofstream
#include <cmath>
#include <boost/functional/hash.hpp>
#include <functional>


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

inline double roundToNearestN(const double value, const unsigned decimal_places) {
	const double multiplier = pow(10, decimal_places);
	return round(value * multiplier) / multiplier;
}

inline sf::VertexArray makeLine(const sf::Vector2f& point1, const sf::Vector2f& point2, const sf::Color color)
{
	sf::VertexArray line(sf::Lines, 2);
	line[0].position = point1;
	line[1].position = point2;
	line[0].color = color;
	line[1].color = color;
	return line;
}


inline std::string formatVariables(const std::vector<std::pair<std::string, double>>& variables) {
	std::ostringstream oss;
	oss.precision(2);
	oss << std::fixed;
	for (const auto& [fst, snd] : variables) {
		oss << fst << ": " << snd << ", ";
	}
	const std::string result = oss.str();
	// Remove the last comma and space
	return result.substr(0, result.size() - 2);
}


inline void drawRectOutline(sf::Rect<float>& rect, sf::RenderWindow& window, const sf::RenderStates& renderStates)
{
	sf::VertexArray lines(sf::Lines, 8);

	// Top line
	lines[0].position = { rect.left, rect.top };
	lines[1].position = { rect.left + rect.width, rect.top };

	// Right line
	lines[2].position = { rect.left + rect.width, rect.top };
	lines[3].position = { rect.left + rect.width, rect.top + rect.height };

	// Bottom line
	lines[4].position = { rect.left + rect.width, rect.top + rect.height };
	lines[5].position = { rect.left, rect.top + rect.height };

	// Left line
	lines[6].position = { rect.left, rect.top + rect.height };
	lines[7].position = { rect.left, rect.top };

	window.draw(lines, renderStates);
}



inline nlohmann::json loadJsonData(const std::string& fileReadWriteName)
{
	std::ifstream ifs(fileReadWriteName);

	if (!ifs.is_open()) {
		std::cerr << "Failed to open the file." << std::endl;
		return nlohmann::json{};
	}

	// Read the content of the JSON file into a JSON object
	nlohmann::json jsonData;
	ifs >> jsonData;

	ifs.close();

	return jsonData;
}


inline sf::Vector2f jsonToVector(const nlohmann::json& jsonPosition)
{
	return {jsonPosition["x"], jsonPosition["y"]};
}

inline nlohmann::json vectorToJson(const sf::Vector2f& vector)
{
	return { {"x", vector.x}, {"y", vector.y} };
}

inline sf::Color jsonToColor(const nlohmann::json& jsonColor)
{
	return { jsonColor["r"], jsonColor["g"], jsonColor["b"], jsonColor["a"]};
}

inline nlohmann::json colorToJson(const sf::Color color)
{
	return { 
	{"r", color.r },
	{"g", color.g },
	{"b", color.b },
	{"a",color.a },
	};
}

inline std::vector<float> jsonToVectorCont(const nlohmann::json& jsonData)
{
	std::vector<float> newData;
	std::cout << jsonData << "\n";
	std::cout << jsonData.size() << "\n";

	for (const auto& i : jsonData)
		newData.push_back(i);
	
	return newData;
}


inline float generaeteUniqueIdentifier(const std::vector<float>& numbers)
{
	std::size_t seed = 0;
	boost::hash_range(seed, numbers.begin(), numbers.end());
	return static_cast<float>(seed) / static_cast<float>(std::numeric_limits<std::size_t>::max());
}


inline float cosineSimilarity(const std::vector<float>& weights1, const std::vector<float>& weights2)
{
	if (weights1.size() != weights2.size())
		return 0.0f;

	float dotProduct = 0.0f;
	float normWeights1 = 0.0f;
	float normWeights2 = 0.0f;

	for (std::size_t i = 0; i < weights1.size(); ++i)
	{
		dotProduct += weights1[i] * weights2[i];
		normWeights1 += weights1[i] * weights1[i];
		normWeights2 += weights2[i] * weights2[i];
	}

	normWeights1 = std::sqrt(normWeights1);
	normWeights2 = std::sqrt(normWeights2);

	if (normWeights1 == 0.0f || normWeights2 == 0.0f)
		return 0.0f;

	return dotProduct / (normWeights1 * normWeights2);
}

inline float generateUniqueIdentifier(const std::vector<float>& weights)
{
	// Calculate the similarity between weights and a reference vector (e.g., average weights)
	std::vector<float> referenceWeights(weights.size(), 0.5f);  // Replace with your reference weights
	float similarity = cosineSimilarity(weights, referenceWeights);

	// Map the similarity value to a unique value between 0 and 1
	return (similarity + 1.0f) / 2.0f;
}