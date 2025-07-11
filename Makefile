# Makefile para o Sistema de Recomendação LSH
# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 
OPENMP_FLAGS = -fopenmp

# Diretórios
SRC_DIR = .
INCLUDE_DIR = ../include
BUILD_DIR = build

# Arquivos fonte
SOURCES = main.cpp recommender_engine.cpp csv_parser.cpp evaluator.cpp
OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Nome do executável
TARGET = recommender

# Regra padrão
all: $(BUILD_DIR) $(TARGET)

# Criar diretório de build se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar o executável
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -o $@ $^

# Regras para compilar objetos
$(BUILD_DIR)/main.o: main.cpp config.hpp types.hpp csv_parser.hpp recommender_engine.hpp evaluator.hpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/recommender_engine.o: recommender_engine.cpp recommender_engine.hpp types.hpp config.hpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/csv_parser.o: csv_parser.cpp csv_parser.hpp types.hpp config.hpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/evaluator.o: evaluator.cpp evaluator.hpp types.hpp config.hpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Limpar arquivos compilados
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Limpar e recompilar
rebuild: clean all

# Executar o programa
run: $(TARGET)
	./$(TARGET)

# Mostrar ajuda
help:
	@echo "Comandos disponíveis:"
	@echo "  make        - Compilar o projeto"
	@echo "  make clean  - Limpar arquivos compilados"
	@echo "  make rebuild- Limpar e recompilar"
	@echo "  make run    - Compilar e executar"
	@echo "  make help   - Mostrar esta ajuda"

# Verificar se OpenMP está disponível
check-openmp:
	@echo "Verificando suporte ao OpenMP..."
	@$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -dM -E - < /dev/null | grep -i openmp || echo "OpenMP não encontrado"

# Instalar dependências (se necessário)
install-deps:
	@echo "Verificando dependências..."
	@which $(CXX) > /dev/null || (echo "Erro: $(CXX) não encontrado. Instale o compilador C++." && exit 1)
	@echo "Compilador $(CXX) encontrado."
	@echo "Dependências verificadas com sucesso!"

.PHONY: all clean rebuild run help check-openmp install-deps 