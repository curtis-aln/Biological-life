#include "Simulation/Simulation.hpp"

/*
 * KEYBINDS
 * shift + c - center
 * shift + v - visual range
 * shift + b - body
 * shift + d - velocities
 * shift + z - zone
 */


int main()
{
	// initilising random
	std::srand(static_cast<unsigned>(time(nullptr)));

	// the color of the simulation is randomly determined by these colors:
	constexpr unsigned colors = 2;
	const sf::Color windowColors[colors] = {
		{20, 30, 50},
		{40, 29, 58}
	};

	const Settings settings(
		1650,
		3'000,
		500,

		true,
		false,

		2500,

		{ 1800, 1000 },
		0.100f,
		2240,
		windowColors[randint(0, colors - 1)],
		"Biologial Evolution Simulation",

		20,
		1'000,

		"data.json",
		{ 25 , 15 } // originally 30, 20
	);

	Simulation simulation(settings);

	simulation.run();
}