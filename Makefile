CXX = mpic++
CXXFLAGS = -Wall -pedantic -Wno-long-long -O0 -ggdb -std=c++11
SRC = $(wildcard src/*.h)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

mpi_compile: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(EXEC)
run:
	$(EXEC)
