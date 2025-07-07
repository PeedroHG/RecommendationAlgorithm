# Nome do compilador
CXX = g++

# Diretórios
SRC_DIR = .
OBJ_DIR = build/objects
BIN = build/app

# Opções de compilação
CXXFLAGS = -Wall -Wextra -O3 -g -fopenmp -I.

# Arquivos fonte
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Arquivos objeto
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Regra padrão
all: $(BIN)

# Compilar o executável
$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compilar objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Executar o programa
run: $(BIN)
	./$(BIN)

# Limpar arquivos de build
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN)

.PHONY: all clean run
