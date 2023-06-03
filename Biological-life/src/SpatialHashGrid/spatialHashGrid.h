#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include <cstdint>
#include <iostream>
/*
	SpatialHashGrid

	Improvements:
	- make a check visual range function
	- make a way to return the cells within visual range
	- automatic resolution resizer

	Notes:
	FINAL GOAL: 50k particles at 144fps
	03/10/2022 - 20k  at 22fps
	26/10/2022 - 40k  at 32fps
	03/03/2023 - 50k  at 25fps
	16/03/2023 - 50k  at 12fps
	25/03/2023 - 100k at 10fps

	REMEMBER:
	changing colors of the rects takes up about 20,000 microseconds
	the reason why cells might not collide instantly might be due to using floats instead of doubles
*/


// https://github.com/johnBuffer/VerletSFML-Multithread/blob/main/src/physics/collision_grid.hpp
struct CollisionCell
{
	// cell_capacity is the absolute MAXIMUM amount of objects that will be in this cell
	static constexpr uint8_t cell_capacity = 20;
	static constexpr uint8_t max_cell_idx = cell_capacity - 1;

	uint8_t objects_count = 0;
	int32_t objects[cell_capacity] = {};

	CollisionCell() = default;

	void addAtom(const int32_t id)
	{
		objects[objects_count] = id;
		objects_count += objects_count < max_cell_idx;
	}

	void clear()
	{
		objects_count = 0;
	}
};


struct c_Vec
{
	static constexpr uint8_t max = CollisionCell::cell_capacity * 9;
	int32_t array[max] = {};
	uint8_t size = 0;

	void add(const int32_t value)
	{
		if (size >= max)
			return;

		array[size] = value;
		size++;
	}

	[[nodiscard]] int32_t at(const unsigned index) const
	{
		return array[index];
	}
};


struct SpatialHashGrid
{
	std::vector<CollisionCell> m_cells{};
	sf::Vector2u m_cellsXY{};

	sf::Vector2f conversionFactor{};
	c_Vec found{};

	// graphics
	sf::Vector2f m_cellDimensions{};
	sf::Rect<float> m_screenSize{};
	sf::VertexBuffer m_renderGrid{};

	// constructor and destructor
	explicit SpatialHashGrid(const sf::Rect<float> screenSize = {}, const sf::Vector2u cellsXY = {})
	{
		init(screenSize, cellsXY);
	}
	~SpatialHashGrid() = default;


	void init(const sf::Rect<float> screenSize, const sf::Vector2u cellsXY)
	{
		m_cellsXY = cellsXY;
		m_screenSize = screenSize;

		m_cells.resize(m_cellsXY.x * m_cellsXY.y);

		m_cellDimensions = { m_screenSize.width / static_cast<float>(m_cellsXY.x),
							m_screenSize.height / static_cast<float>(m_cellsXY.y) };

		conversionFactor = { 1.f / m_cellDimensions.x, 1.f / m_cellDimensions.y };

		initVertexBuffer();
	}


	// other functions
	void addAtom(const sf::Vector2f pos, const int32_t atom)
	{
		const sf::Vector2<uint32_t> cIdx = posTo2dIdx(pos);

		if (!checkValidIndex(cIdx))
			throw std::out_of_range("position argument out of range");

		const uint32_t idx = idx2dTo1d(cIdx);
		m_cells[idx].addAtom(atom);
	}

	void clear()
	{
		for (CollisionCell& cell : m_cells) {
			cell.objects_count = 0;
		}
	}

	c_Vec& find(const sf::Vector2f position)
	{
		found.size = 0;

		const sf::Vector2<uint32_t> cIdx = posTo2dIdx(position);
		if (!checkValidIndex(cIdx))
			throw std::out_of_range("find() position argument out of range");

		// getting the indexes needed
		for (unsigned x = cIdx.x - 1; x <= cIdx.x + 1; x++)
		{
			for (unsigned y = cIdx.y - 1; y <= cIdx.y + 1; y++)
			{
				const CollisionCell& cell = m_cells[idx2dTo1d({ x, y })];

				for (unsigned i{0}; i < cell.objects_count; i++)
					found.add(cell.objects[i]);
			}
		}

		return found;
	}

	[[nodiscard]] uint32_t idx2dTo1d(const sf::Vector2<uint32_t> idx) const
	{
		return idx.x + idx.y * m_cellsXY.x;
	}

	[[nodiscard]] sf::Vector2<uint32_t> posTo2dIdx(const sf::Vector2f position) const
	{
		return {
			static_cast<uint32_t>(position.x * conversionFactor.x),
			static_cast<uint32_t>(position.y * conversionFactor.y)};
	}

	[[nodiscard]] bool checkValidIndex(const sf::Vector2u index) const
	{
		return !(index.x < 0 || index.y < 0 || index.x >= m_cellsXY.x || index.y >= m_cellsXY.y);
	}


	void initVertexBuffer()
	{
		std::vector<sf::Vertex> vertices(static_cast<std::vector<sf::Vertex>::size_type>((m_cellsXY.x + m_cellsXY.y) * 2));

		m_renderGrid = sf::VertexBuffer(sf::Lines, sf::VertexBuffer::Static);
		m_renderGrid.create(vertices.size());

		size_t counter = 0;
		for (unsigned i = 0; i < m_cellsXY.x; i++)
		{
			const float posX = static_cast<float>(i) * m_cellDimensions.x;
			vertices[counter].position = { posX, 0 };
			vertices[counter + 1].position = { m_screenSize.left + posX, m_screenSize.top + m_screenSize.height };
			counter += 2;
		}

		for (unsigned i = 0; i < m_cellsXY.y; i++)
		{
			const float posY = static_cast<float>(i) * m_cellDimensions.y;
			vertices[counter].position = { 0, posY };
			vertices[counter + 1].position = { m_screenSize.left + m_screenSize.width, m_screenSize.top + posY };
			counter += 2;
		}

		m_renderGrid.update(vertices.data(), vertices.size(), 0);
	}


	void reSize(const sf::Rect<float> screenSize)
	{
		init(screenSize, m_cellsXY);
	}
};