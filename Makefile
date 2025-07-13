# Makefile para o Sistema de Recomendação LSH
# ==========================================

# --- Configuração do Compilador ---
CXX = g++
# Aponta o include para a pasta 'src', onde estão os headers (.hpp)
INCLUDE_DIR = src
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -flto -ffast-math -I$(INCLUDE_DIR)
OPENMP_FLAGS = -fopenmp
# Flags para a etapa de linkagem (junção dos arquivos .o)
LDFLAGS = $(OPENMP_FLAGS) -flto

# --- Estrutura de Diretórios ---
SRC_DIR = src
BUILD_DIR = build

# --- Arquivos do Projeto ---
# Encontra automaticamente todos os arquivos .cpp dentro de SRC_DIR
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Gera os nomes dos arquivos objeto (.o) correspondentes na pasta BUILD_DIR
# Ex: src/main.cpp -> build/main.o
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
# Nome do executável final
TARGET = recommender

# --- Regras Principais ---

# Regra padrão: executada ao rodar 'make' ou 'make all'
# O executável final é o nosso objetivo principal.
all: $(TARGET)

# Regra de linkagem: cria o executável a partir dos arquivos objeto.
# '$@' é o nome do alvo (recommender)
# '$^' são todas as dependências (a lista de arquivos .o)
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Regra de Padrão para compilação: transforma qualquer .cpp de 'src' em um .o em 'build'.
# '$<' é a primeira dependência (o arquivo .cpp correspondente)
# O '| $(BUILD_DIR)' é uma dependência de ordem apenas, garante que o diretório exista antes de compilar.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -c $< -o $@

# Regra para criar o diretório de build.
# Só é executada se o diretório não existir.
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


# --- Comandos Utilitários ---

# Limpa todos os arquivos gerados pela compilação.
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)

# Limpa e compila tudo novamente.
rebuild: clean all

# Compila (se necessário) e executa o programa.
run: all
	./$(TARGET)

# Mostra uma ajuda amigável.
help:
	@echo "Comandos disponíveis:"
	@echo "  make ou make all - Compila o projeto"
	@echo "  make clean       - Limpa os arquivos compilados (pasta 'build' e o executável)"
	@echo "  make rebuild     - Limpa e recompila o projeto do zero"
	@echo "  make run         - Compila (se necessário) e executa o programa"
	@echo "  make help        - Mostra esta mensagem de ajuda"

# Declara alvos que não são nomes de arquivos, para evitar conflitos.
.PHONY: all clean rebuild run help

make r: clean make run