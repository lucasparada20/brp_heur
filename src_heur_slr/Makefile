CXX := g++
CXXFLAGS := -O3 -Wall

TARGET := exec_heur

# Source files
SRC := main_heur_brp.cpp \
       AlnsBRP.cpp \
       BestSolutionList.cpp \
       Parameters.cpp \
       LoadBRP.cpp \
       CostFunctionBRP.cpp \
       SeqInsertBRP_HC.cpp \
       SeqInsertBRP_HR.cpp \
       InsRmvMethodBRP.cpp \
       RouteFeasibility.cpp \
       RandomRemoveBRP.cpp \
       RelatednessRemoveBRP.cpp

# Object files
OBJ := $(SRC:.cpp=.o)

# Build rule
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Object file rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
