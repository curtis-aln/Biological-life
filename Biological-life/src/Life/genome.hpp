#pragma once

#include "SFML/Graphics.hpp"
#include "../utility.hpp"
#include <array>
#include <nlohmann/json.hpp>

#include <vector>
#include <cmath>
#include <random>

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

    

public:
	explicit Genome() { createRandomMutations(*this); }


    // Quick functions
    [[nodiscard]] sf::Color& getColor()      { return m_color; }
    [[nodiscard]] float getGenomeRadius() const    { return m_mass * massScale; }
    [[nodiscard]] float getSpeedMax() const  { return m_maxSpeed * speedScale; }
    [[nodiscard]] float getRawMass() const   { return m_mass; }



	static void createRandomMutations(Genome& genome)
	{
        // mutating genetic codes
        genome.m_mutationRate = randfloat(mutationRateR.x, mutationRateR.y);
        genome.m_mass = randfloat(massR.x, massR.y);
        genome.m_maxSpeed = randfloat(speedR.x, speedR.y);
        genome.m_color = randColor();
        genome.m_color.a = 85;
    }


    void createMutatedClone(Genome& genome) const
    {
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
            {"color", {"r", m_color.r}, {"g", m_color.g}, {"b", m_color.b}},
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
            static_cast<sf::Uint8>(m_color.b + randint(colorMR, -colorMR)),
            85,
        };
    }


    static void randCodes(std::array<float, GeneticCodeCount>& geneticCodes)
    {
        for (unsigned int i{0}; i < GeneticCodeCount; i++)
            geneticCodes[i] = randfloat(geneticCodeR.x, geneticCodeR.y);
    }

};




class Perceptron
{
public:
    // Constructor takes number of input nodes and number of output nodes
    explicit Perceptron(const unsigned num_inputs, const unsigned num_outputs)
        : m_numInputs(num_inputs), m_numOutputs(num_outputs)
    {
        // Initialize weights to random values between -1 and 1
        m_weights.resize(m_numInputs * m_numOutputs);
        for (unsigned i = 0; i < m_numInputs * m_numOutputs; ++i)
            m_weights[i] = getRandWeight();

        weightedOutputs.resize(m_numOutputs);
    }

    // Compute output of the perceptron for a given set of inputs
    void compute_output(const std::vector<float>& inputs)
    {
        // Check that the number of inputs matches the number of weights
        assert(inputs.size() == m_numInputs);

        // Compute the weighted sum of the inputs for each output node
        for (unsigned i = 0; i < m_numOutputs; ++i)
        {
            float weighted_sum = 0.f;
            for (unsigned j = 0; j < m_numInputs; ++j)
            {
                const unsigned weight_idx = i * m_numInputs + j;
                weighted_sum += inputs[j] * m_weights[weight_idx];
            }

            float output = 1.f / (1.f + exp(-weighted_sum));
            // Scale the output value to the range [-1, 1]
            output = 2.f * output - 1.f;
            weightedOutputs[i] = output;
        }
    }

    // Mutate the weights of the perceptron
    void mutate(Perceptron& perceptronToMutate) const
    {
        // Iterate over the weights and mutate each one with a certain probability
        for (unsigned i = 0; i < m_numInputs * m_numOutputs; ++i)
        {
            if (randfloat(0.f, 1.f) < m_mutationRate)
                perceptronToMutate.m_weights[i] = m_weights[i] + getRandWeight() * m_mutationRange;
        }
    }

    nlohmann::json saveNetworkJson()
    {
        return {
        	{"num inputs", m_numInputs},
            {"num outputs", m_numOutputs},
            {"weights", m_weights}

        };
    }

private:
    static float getRandWeight() { return randfloat(-1.f, 1.f); }

    const unsigned m_numInputs;    // Number of input nodes
    const unsigned m_numOutputs;   // Number of output nodes
    std::vector<float> m_weights;  // Weight values

    const float m_mutationRate = 0.35f;
    const float m_mutationRange = 0.5f;


public:
    // pre-making the output container
    std::vector<float> weightedOutputs;
};
