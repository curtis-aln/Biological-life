#include "Simulation.hpp"

void Simulation::initStatisticVariables()
{
	cellPopulation.reserve(20'000);
	plantPopulation.reserve(20'000);
	avgReproCount.reserve(20'000);
	avgLifeTime.reserve(20'000);

	cellPopulation.push_back(0);
	plantPopulation.push_back(0);
	avgReproCount.push_back(0);
	avgLifeTime.push_back(0);
}

void Simulation::saveData()
{
	nlohmann::json entityData;
	for (Cell* cell : m_Cells)
		entityData.push_back(cell->saveCellJson());

	//for (const unsigned index : m_Plants.getAvalableIndexes())
	//	entityData.push_back(m_Plants.at(index).savePlantJson());


	const nlohmann::json simulationData = {
		{"total frame count", totalFrameCount},
		{"total extinctions", totalExtinctions},
		{"total run time", totalRunTime},
		{"entities", entityData}
	};

	std::ofstream ofs(fileReadWriteName);
	ofs << simulationData.dump(3); // 4 is the number of spaces for indentation
	ofs.close();
}


void Simulation::loadData()
{
	clearEntityData();

	nlohmann::json simulationData = loadJsonData(fileReadWriteName);

	// setting the current simulation information
	totalFrameCount = simulationData["total frame count"];
	totalExtinctions = simulationData["total extinctions"];
	totalRunTime = simulationData["total run time"];

	unsigned i = 0;
	for (const nlohmann::json& cellData : simulationData["entities"])
	{
		Cell* newCell = m_Cells.add();
		if (newCell == nullptr)
			break;

		newCell->loadCellData(cellData);
		bufferPosUpdate({ newCell->indexes }, newCell->getDeltaPos());
		bufferColorUpdate({ newCell->indexes }, newCell->getColor());
		i++;
	}

	alignCells();

	plantUnderflowProtection(initPlantCount);
}


void Simulation::printStatistics()
{
	std::cout << "-------------------------------------------------- " << ++updateCounter << "\n";
	std::cout << "Total Alive : " << m_Cells.size() << " Cells, " << m_Plants.size() << " Plants" << "\n";
	std::cout << "Total Frames: " << totalFrameCount    << "\n";
	std::cout << "Rel Frames  : " << relativeFrameCount << "\n";
	std::cout << "Extinctions : " << totalExtinctions   << "\n";
	std::cout << "Avg repro   : " << roundToNearestN(static_cast<double>(avgReproCount.at(avgReproCount.size() - 1)), 1)  << "\n";
	std::cout << "Avg age     : " << roundToNearestN(static_cast<double>(avgLifeTime.at(avgLifeTime.size() - 1)), 1)    << "\n";
	std::cout << "Time Passed : " << roundToNearestN(totalRunTime / 60, 2)   << " mins \n";
	std::cout << "            : " << roundToNearestN(totalRunTime / 3600, 2) << " hours \n";
	std::cout << "\n";
}


void Simulation::updateStatistics()
{
	constexpr unsigned printFreq = 1'000;
	constexpr unsigned saveFreq = 2'000;

	if (totalFrameCount % printFreq == 0 && !m_paused)
	{
		// updating the current statistics
		cellPopulation.push_back(m_Cells.size());
		plantPopulation.push_back(m_Plants.size());
		updateCellStatistics();

		printStatistics();
	}

	//if (totalFrameCount % 2000 == 0 && minPlants > 35 && m_Cells.size() > 9900)
	//{
	//	minPlants -= 1;
	//	std::cout << "min plants: " << minPlants << "\n";
	//}

	if (totalFrameCount % saveFreq == 0 && !m_paused && m_autoSaving)
	{
		std::cout << "Autosaving. . ." << "\n";
		saveData();
	}
}