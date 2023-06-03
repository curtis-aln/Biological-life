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
		{40, 29, 58},
		//{8, 12, 7},
		//{208, 196, 230},
		//{10, 32, 30},
		//{0, 0, 0},
	};

	const Settings settings(
		2'000,
		3'000,
		500,
		true,
		false,
		{ 1800, 1000 },
		.12f,
		13'000,
		4'000,
		20,
		500,
		windowColors[randint(0, colors-1)],
		"Biologial Evolution Simulation",
		"data.json",
		{ 30 , 20 }
	);

	Simulation simulation(settings);

	simulation.run();
}