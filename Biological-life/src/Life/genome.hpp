#pragma once

#include "SFML/Graphics.hpp"
#include "../utility.hpp"
#include <nlohmann/json.hpp>

#include <vector>
#include <cmath>
#include <cassert>
#include "../settings.hpp"


class Genome : GenomeSettings
{
public:
	explicit Genome() = default;

	// Quick functions
    [[nodiscard]] static sf::Color& randCellColor()
	{
		sf::Color color = randColor();
		color.a = 85;
        return color;
	}
    [[nodiscard]] static float randRadius() { return randfloat(0.1f, 1.f); }


protected:
	[[nodiscard]] static sf::Color createMutatedColor(const sf::Color& colorRef)
	{
        return {
            static_cast<sf::Uint8>(colorRef.r + randint(colorMR, -colorMR)),
            static_cast<sf::Uint8>(colorRef.g + randint(colorMR, -colorMR)),
            static_cast<sf::Uint8>(colorRef.b + randint(colorMR, -colorMR)),
            85,
        };
    }
};


class Perceptron
{
public:
    // Constructor takes number of input nodes, number of hidden layers, size of each hidden layer, and number of output nodes
    explicit Perceptron(const unsigned num_inputs = 0, const unsigned num_hidden_layers = 0, 
        const unsigned hidden_layer_size = 0, const unsigned num_outputs = 0)
        : m_numInputs(num_inputs), m_numHiddenLayers(num_hidden_layers), m_hiddenLayerSize(hidden_layer_size), m_numOutputs(num_outputs)
    {
        // Initialize weights for the connections between input layer and first hidden layer
        m_weightsInputHidden.resize(m_numInputs * m_hiddenLayerSize);
        for (unsigned i = 0; i < m_numInputs * m_hiddenLayerSize; ++i)
            m_weightsInputHidden[i] = getRandWeight();

        // Initialize weights for the connections between hidden layers
        m_weightsHiddenHidden.resize((m_numHiddenLayers - 1) * m_hiddenLayerSize * m_hiddenLayerSize);
        for (unsigned i = 0; i < (m_numHiddenLayers - 1) * m_hiddenLayerSize * m_hiddenLayerSize; ++i)
            m_weightsHiddenHidden[i] = getRandWeight();

        // Initialize weights for the connections between last hidden layer and output layer
        m_weightsHiddenOutput.resize(m_hiddenLayerSize * m_numOutputs);
        for (unsigned i = 0; i < m_hiddenLayerSize * m_numOutputs; ++i)
            m_weightsHiddenOutput[i] = getRandWeight();

        weightedOutputs.resize(m_numOutputs);
    }

    Perceptron& operator=(const Perceptron& other)
    {
        if (this == &other)
            return *this;  // Check for self-assignment

        m_numInputs           = other.m_numInputs;
    	m_numHiddenLayers     = other.m_numHiddenLayers;
    	m_hiddenLayerSize     = other.m_hiddenLayerSize;
    	m_numOutputs          = other.m_numOutputs;
        m_weightsInputHidden  = other.m_weightsInputHidden;
        m_weightsHiddenHidden = other.m_weightsHiddenHidden;
        m_weightsHiddenOutput = other.m_weightsHiddenOutput;
        m_mutationRate        = other.m_mutationRate;
        m_mutationRange       = other.m_mutationRange;

        // Pre-making the output container
        std::vector<float> weightedOutputs;

        return *this;
    }

public:
    // Compute output of the perceptron for a given set of inputs
    void compute_output(const std::vector<float>& inputs)
    {
        // Check that the number of inputs matches the number of weights for the input layer
        assert(inputs.size() == m_numInputs);

        // Compute the weighted sum of the inputs for each hidden layer node
        std::vector<float> hiddenLayerInputs(m_hiddenLayerSize, 0.f);
        for (unsigned i = 0; i < m_hiddenLayerSize; ++i)
        {
            for (unsigned j = 0; j < m_numInputs; ++j)
            {
                const unsigned weight_idx = i * m_numInputs + j;
                hiddenLayerInputs[i] += inputs[j] * m_weightsInputHidden[weight_idx];
            }

            hiddenLayerInputs[i] = sigmoid(hiddenLayerInputs[i]);
        }

        // Compute the weighted sum of the inputs for each hidden layer except the first one
        std::vector<float> hiddenLayerOutputs(hiddenLayerInputs);
        for (unsigned layer = 1; layer < m_numHiddenLayers; ++layer)
        {
            std::vector<float> newHiddenLayerOutputs(m_hiddenLayerSize, 0.f);
            for (unsigned i = 0; i < m_hiddenLayerSize; ++i)
            {
                for (unsigned j = 0; j < m_hiddenLayerSize; ++j)
                {
                    const unsigned weight_idx = (layer - 1) * m_hiddenLayerSize * m_hiddenLayerSize + i * m_hiddenLayerSize + j;
                    newHiddenLayerOutputs[i] += hiddenLayerOutputs[j] * m_weightsHiddenHidden[weight_idx];
                }

                newHiddenLayerOutputs[i] = sigmoid(newHiddenLayerOutputs[i]);
            }

            hiddenLayerOutputs = newHiddenLayerOutputs;
        }

        // Compute the weighted sum of the inputs for each output node
        for (unsigned i = 0; i < m_numOutputs; ++i)
        {
            float weighted_sum = 0.f;
            for (unsigned j = 0; j < m_hiddenLayerSize; ++j)
            {
                const unsigned weight_idx = j * m_numOutputs + i;
                weighted_sum += hiddenLayerOutputs[j] * m_weightsHiddenOutput[weight_idx];
            }

            float output = sigmoid(weighted_sum);
            // Scale the output value to the range [-1, 1]
            output = 2.f * output - 1.f;
            weightedOutputs[i] = output;
        }
    }

    // Mutate the weights of the perceptron
    void mutate(Perceptron& perceptronToMutate) const
    {
        // Iterate over the weights for the input layer and mutate each one with a certain probability
        for (unsigned i = 0; i < m_numInputs * m_hiddenLayerSize; ++i)
        {
            if (randfloat(0.f, 1.f) < m_mutationRate)
                perceptronToMutate.m_weightsInputHidden[i] = m_weightsInputHidden[i] + getRandWeight() * m_mutationRange;
        }

        // Iterate over the weights for the connections between hidden layers and mutate each one with a certain probability
        for (unsigned i = 0; i < (m_numHiddenLayers - 1) * m_hiddenLayerSize * m_hiddenLayerSize; ++i)
        {
            if (randfloat(0.f, 1.f) < m_mutationRate)
                perceptronToMutate.m_weightsHiddenHidden[i] = m_weightsHiddenHidden[i] + getRandWeight() * m_mutationRange;
        }

        // Iterate over the weights for the connections between the last hidden layer and output layer and mutate each one with a certain probability
        for (unsigned i = 0; i < m_hiddenLayerSize * m_numOutputs; ++i)
        {
            if (randfloat(0.f, 1.f) < m_mutationRate)
                perceptronToMutate.m_weightsHiddenOutput[i] = m_weightsHiddenOutput[i] + getRandWeight() * m_mutationRange;
        }
    }

    nlohmann::json saveNetworkJson()
    {
        return {
            {"num inputs", m_numInputs},
            {"num hidden layers", m_numHiddenLayers},
            {"hidden layer size", m_hiddenLayerSize},
            {"num outputs", m_numOutputs},
            {"weights input-hidden", m_weightsInputHidden},
            {"weights hidden-hidden", m_weightsHiddenHidden},
            {"weights hidden-output", m_weightsHiddenOutput}
        };
    }

    void loadNetworkData(const nlohmann::json& networkData)
    {
        m_numInputs = networkData["num inputs"];
        m_numHiddenLayers = networkData["num hidden layers"];
        m_hiddenLayerSize = networkData["hidden layer size"];
        m_numOutputs = networkData["num outputs"];

        m_weightsInputHidden.clear();
        for (const auto& i : networkData["weights input-hidden"])
	        m_weightsInputHidden.push_back(i);

        m_weightsHiddenHidden.clear();
        for (const auto& i : networkData["weights hidden-hidden"])
	        m_weightsHiddenHidden.push_back(i);

        m_weightsHiddenOutput.clear();
        for (const auto& i : networkData["weights hidden-output"])
	        m_weightsHiddenOutput.push_back(i);
    }

protected:
    std::vector<float>& getInputHiddenWeights() { return m_weightsInputHidden; }

private:
    static float getRandWeight() { return randfloat(-1.f, 1.f); }

    static float sigmoid(const float x) { return 1.f / (1.f + std::exp(-x)); }

    unsigned m_numInputs;             // Number of input nodes
    unsigned m_numHiddenLayers;       // Number of hidden layers
    unsigned m_hiddenLayerSize;       // Size of each hidden layer
    unsigned m_numOutputs;            // Number of output nodes

    std::vector<float> m_weightsInputHidden;      // Weight values for the connections between input layer and first hidden layer
    std::vector<float> m_weightsHiddenHidden;     // Weight values for the connections between hidden layers
    std::vector<float> m_weightsHiddenOutput;     // Weight values for the connections between last hidden layer and output layer

	float m_mutationRate  = 0.30f;
	float m_mutationRange = 0.24f;

public:
    // Pre-making the output container
    std::vector<float> weightedOutputs;
};
