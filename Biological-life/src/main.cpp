#include "Simulation/Simulation.hpp"


/*
 * Quality of life features
 * - different background colors every time (at least 20), keep them mundane so they contrast well
 * - simplistic patterns with vertex buffer such as circles inside of circles. this can be done by adding a feature for custom
 *   allocation / custom object creation
 *
 */

/*
 * KEYBINDS
 * shift + c - center
 * shift + v - visual range
 * shift + b - body
 * shift + d - velocities
 * shift + z - zone
 */

/*
 *
 * optimizations
 * 1. 1:13
 * 2. 
 */

int main()
{
	// initilising random
	std::srand(static_cast<unsigned>(time(nullptr)));

	
	const Settings settings(
		300,
		4000,
		500,
		true,
		{ 1800, 1000 },
		.27f,
		4000,
		1000,
		20,
		30,
		{ 20, 30, 50 },
		"Biologial Evolution Simulation",
		"data.json",
		{ 30 , 20 }
	);

	Simulation simulation(settings);

	simulation.run();
}