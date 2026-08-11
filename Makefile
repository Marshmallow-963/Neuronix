CC := gcc

CFG_DIR := config
INC_DIR := include
SRC_DIR := src
LIB_DIR := lib
OBJ_DIR := obj
BIN_DIR := bin

# --- Novas variáveis para os Assets ---
ASSETS_DIR := assets/img
GEN_INC_DIR := $(INC_DIR)/generated
QOI_FILES := $(shell find $(ASSETS_DIR) -name "*.qoi")
# Transforma os caminhos de .qoi para .h
HEADER_FILES := $(patsubst $(ASSETS_DIR)/%.qoi, $(GEN_INC_DIR)/%.h, $(QOI_FILES))

TARGET := $(BIN_DIR)/neurolab

CFLAGS  := -Wall -Wextra -Wconversion -std=c99 -fsanitize=address -g -O2 -DRAYGUI_SUPPORT_ICONS
LDFLAGS := -fsanitize=address -L$(LIB_DIR)/linux -lraylib -lsqlite3 -lm -lpthread -ldl -lrt -lX11

SOURCES := $(shell find $(SRC_DIR) -name "*.c")
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Adicionei a pasta generated no include path
CPPFLAGS := -I$(CFG_DIR) -I$(INC_DIR) -I$(GEN_INC_DIR) -I$(LIB_DIR) -MMD -MP

# Faça o 'all' depender da geração dos headers antes de compilar o executável
all: $(HEADER_FILES) $(TARGET)

$(GEN_INC_DIR)/%.h: $(ASSETS_DIR)/%.qoi
	@echo "==> Gerando header para: $<"
	@mkdir -p $(@D)
	@# Usamos aspas para proteger contra espaços nos diretórios.
	@# $(*F) pega apenas o nome do arquivo sem caminho e sem extensão.
	xxd -i -n $(*F)_qoi "$<" "$@"

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@# --- Criação dos Links Simbólicos ---
	@rm -rf $(@D)/assets
	@ln -sf ../assets $(@D)/assets
	@rm -rf $(@D)/data
	@ln -sf ../data $(@D)/data
	@echo "=> Compilação concluída com sucesso! Executável em: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "==> Compilando: $<"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) $(GEN_INC_DIR)

-include $(OBJECTS:.o=.d)

run: all
	@LSAN_OPTIONS=suppressions=lsan.supp ./neurolab.sh

.PHONY: all clean run
