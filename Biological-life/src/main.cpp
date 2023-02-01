#include "Simulation.hpp"

int main()
{
	// initilising random
	std::srand(static_cast<unsigned>(time(NULL)));

	// settings
	constexpr OrganicSettings simulationSettings(
		30, 30, 60, true
	);

	const renderingSettings renderSettings(
		1800, 1000, { 20, 30, 50 }
	);


	Simulation simulation(simulationSettings, renderSettings);

	simulation.run();
}