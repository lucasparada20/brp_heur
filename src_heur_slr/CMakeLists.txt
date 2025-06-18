cmake_minimum_required(VERSION 3.2)
project(ExecHeurSlr)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
	main_heur_brp.cpp
	AlnsBRP.cpp
	BestSolutionList.cpp
	Parameters.cpp
	LoadBRP.cpp
	CostFunctionBRP.cpp
	SeqInsertBRP_HC.cpp
	SeqInsertBRP_HR.cpp
	InsRmvMethodBRP.cpp
	RouteFeasibility.cpp
	RandomRemoveBRP.cpp
	RelatednessRemoveBRP.cpp
)

# Target: exec_heur
add_executable(exec_heur ${SOURCES})

