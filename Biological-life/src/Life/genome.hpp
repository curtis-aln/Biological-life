#pragma once

#include "SFML/Graphics.hpp"
#include "../utility.hpp"
#include <array>
#include <nlohmann/json.hpp>

class GenomeSettings
{
protected:
    inline static constexpr unsigned int GeneticCodeCount = 30;
    inline static constexpr unsigned int CodeMutationCount = 6;
    inline static constexpr unsigned int pointerDelay = 2;
    

    // scale factors
    inline static constexpr float massScale = 1.0f;
    inline static constexpr float speedScale = 1.f;

    // mutation and gene related (mr = mutation rate)
    inline static constexpr float changeInMR = 0.00'09f;
    inline static constexpr int colorMR = 4;

    // ranges
    //inline static const sf::Vector2f massR = { 7.5f, 20.f };
    inline static const sf::Vector2f massR = { 10.f, 10.f };
    inline static const sf::Vector2f speedR = { 0.f, 1.f };
    inline static const sf::Vector2f mutationRateR = { 0.00'05f, 0.00'40f };
	inline static const sf::Vector2f geneticCodeR = { -0.02f, 0.06f };
};



class Genome : GenomeSettings
{
	float m_mass{};
	float m_maxSpeed{};
	float m_mutationRate{};

    sf::Color m_color;

    unsigned m_internalClock = 0;

    // genetic code
    unsigned int m_pointer = 0;
    std::array<float, GeneticCodeCount> m_geneticCodes = {};
    

public:
	explicit Genome() { createRandomMutations(*this); }


    // Quick functions
    [[nodiscard]] sf::Color& getColor()      { return m_color; }
    [[nodiscard]] float getGenomeRadius() const    { return m_mass * massScale; }
    [[nodiscard]] float getSpeedMax() const  { return m_maxSpeed * speedScale; }
    [[nodiscard]] float getRawMass() const   { return m_mass; }


    float calculateInteration()
	{
        m_internalClock++;
        if (m_internalClock % pointerDelay == 0) m_pointer++;
        if (m_pointer >= m_geneticCodes.size() - 1) m_pointer = 0;

        return m_geneticCodes[m_pointer];
	}


	static void createRandomMutations(Genome& genome)
	{
        // mutating genetic codes
        genome.m_mutationRate = randfloat(mutationRateR.x, mutationRateR.y);
        genome.m_mass = randfloat(massR.x, massR.y);
        genome.m_maxSpeed = randfloat(speedR.x, speedR.y);
        genome.m_color = randColor();
        randCodes(genome.m_geneticCodes);
    }


    void createMutatedClone(Genome& genome) const
    {
        // mutating genetic codes
        for (unsigned int _{ 0 }; _ < CodeMutationCount; _++)
        {
            const size_t i = randint(0, static_cast<int>(genome.m_geneticCodes.size()) - 1);
            genome.m_geneticCodes[i] = m_geneticCodes[i] + randMutation();
        }

        // mutating mass and speed
        // TODO: dont forget to uncomment
        //genome.m_mass = m_mass + randMutation();
        genome.m_maxSpeed = m_maxSpeed + randMutation();
        genome.m_mutationRate = m_mutationRate + randfloat(mutationRateR.x, mutationRateR.y);
        genome.m_color = createMutatedColor();
	}

    nlohmann::json saveGenomeData()
	{
		return {
            {"mass", m_mass},
            {"max speed", m_maxSpeed},
            {"mutation rate", m_mutationRate},
            {"internal clock", m_internalClock},
            {"m_pointer", m_pointer},
            {"color", {"r", m_color.r}, {"g", m_color.g}, {"b", m_color.b}},
            {"codes", m_geneticCodes}
        };
	}



private:
    [[nodiscard]] float randMutation() const
    {
        return randfloat(m_mutationRate, m_mutationRate * -1);
    }

	[[nodiscard]] sf::Color createMutatedColor() const
    {
        return {
            static_cast<sf::Uint8>(m_color.r + randint(colorMR, -colorMR)),
            static_cast<sf::Uint8>(m_color.g + randint(colorMR, -colorMR)),
            static_cast<sf::Uint8>(m_color.b + randint(colorMR, -colorMR))
        };
    }


    static void randCodes(std::array<float, GeneticCodeCount>& geneticCodes)
    {
        for (unsigned int i{0}; i < GeneticCodeCount; i++)
            geneticCodes[i] = randfloat(geneticCodeR.x, geneticCodeR.y);
    }

};