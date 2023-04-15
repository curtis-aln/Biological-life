#pragma once
#include <SFML/Graphics.hpp>

/*
 * TODO:
 * - apon generation, turn all of the vertecies into circle molds so they can be processed
 * - fix the removal bug
 *
 * Update optimisation:
 * - keep track of what verticies indexes have been updated up until the user calles the update function
 * - then clear the container
 */


 /* An Allocations is a class that manipulates Vertices inside of the VertexBuffer */
struct Allocations
{
	std::vector<unsigned int> indexes;
};


class Buffer
{
	const unsigned int m_maxObjects;
	const unsigned int m_ObjectPoints;
	const unsigned int m_verticesMultiplier;
	const double PI = 3.14159265358979;

	unsigned int m_totalExpectedVertices;

	sf::VertexBuffer m_VertexBuffer;
	std::vector<sf::Vertex> m_vertices;
	std::vector<unsigned int> m_verticesIndexes;

	// variable used for keeping track of all of the allocations issued and recived
	unsigned int m_allocationsIssued = 0;


public:
	// constructor and detructor
	explicit Buffer(unsigned maxObjects, unsigned objectPoints, sf::VertexBuffer::Usage usage = sf::VertexBuffer::Stream);
	~Buffer() = default;

	[[nodiscard]] Allocations add(sf::Vector2f position = {0, 0}, float radius = 0.0, sf::Color color = { 0, 0, 0 });

	std::vector<sf::Vertex>* getVertices() { return &m_vertices; }
	sf::VertexBuffer* getBuffer() { return &m_VertexBuffer; }

	void remove(const Allocations* object);
	void render(sf::RenderTarget* renderTarget) const;
	void update();

	// allocation processing
	void setVertexPositions(const Allocations& allocations, sf::Vector2f deltaPosition);
	void scaleObject(const Allocations& allocations, sf::Vector2f centerPoint, float scaleFactor);
	void setColor(const Allocations& allocations, sf::Color newColor);

private:
	Allocations handleOnePointPrimitive(sf::Vector2f position, sf::Color color);
	Allocations handleFourPointPrimitive(sf::Vector2f position, sf::Color color, float size);
	Allocations handleThreePointPrimitive(sf::Vector2f position, sf::Color color, float size);

	void addToVertexVector(Allocations& object, sf::Color color, const std::vector<sf::Vertex>& vertices);
	void overflowManagement() const;

	[[nodiscard]] std::vector<sf::Vertex> createTriangleVertices(float radius, sf::Vector2f position) const;
	[[nodiscard]] std::vector<sf::Vertex> createSquare(sf::Vector2f position, float size) const;
	[[nodiscard]] std::vector<sf::Vertex> createTriangleAroundPoint(sf::Vector2f position, float size) const;
	[[nodiscard]] sf::Vector2f idxToCoords(unsigned int idx, float radius) const;
	[[nodiscard]] unsigned int scaleIndex(unsigned int index, bool scaleUp) const;
	[[nodiscard]] static sf::PrimitiveType getPrimitiveType(unsigned int objectPoints);
	[[nodiscard]] static unsigned int getMultiplier(unsigned int objectPoints);
	[[nodiscard]] unsigned int getNextIndex();
};

