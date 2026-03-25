#------------------------------------------------------------------------------
# MAKEFILE : Lecroy Binary Read Software
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Defining the directory structure and some flags.
#------------------------------------------------------------------------------
# Define output directories
OBJ_DIR = obj
BIN_DIR = bin
DEP_DIR = dep

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR))

CFLAGS = $(shell root-config --cflags)
LIBS   = $(shell root-config --libs)
GLIBS  = $(shell root-config --glibs)

# Add include paths
INCLUDES = -I$(ROOTSYS)/include

# Programmes to be linked.
PROGS = bin2tree

# Source files
SOURCES = $(PROGS:=.cxx)

# Object files
OBJECTS = $(addprefix $(OBJ_DIR)/, $(PROGS:=.o))

# Dependency files
DEPENDS = $(addprefix $(DEP_DIR)/, $(PROGS:=.d))

#------------------------------------------------------------------------------
# Compiler and linker flags
#------------------------------------------------------------------------------
CXX = g++
CXXFLAGS = $(CFLAGS) $(INCLUDES) -Wall -Wextra -g
LDFLAGS = $(LIBS) $(GLIBS) -L$(ROOTSYS)/lib -lCore -lTree -ldl -lRIO -lNet -lHist

#------------------------------------------------------------------------------
# Defining the compiling targets.
#------------------------------------------------------------------------------
all: Wellcome directories $(addprefix $(BIN_DIR)/, $(PROGS))

#------------------------------------------------------------------------------
# Create directories
#------------------------------------------------------------------------------
directories:
	mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR)

#------------------------------------------------------------------------------
# Wellcome message.
#------------------------------------------------------------------------------
Wellcome:
	@echo -e "\033[40m\033[0;1;32m ------------------------------ \033[0m"
	@echo -e "\033[40m\033[0;1;32m  Lecroy Read Makefile          \033[0m"
	@echo -e "\033[40m\033[0;1;32m  New Version: 25th March 2025  \033[0m"
	@echo -e "\033[40m\033[0;1;32m ------------------------------ \033[0m"
	@echo -e "\033[47m\033[0;1;33m ------------------------------ \033[0m"
	@echo -e "\033[47m\033[0;1;33m  T. Papaevangelou  - A.M. Altingun \033[0m"
	@echo -e "\033[47m\033[0;1;33m ------------------------------ \033[0m"
	@echo

#------------------------------------------------------------------------------
# Build executables
#------------------------------------------------------------------------------
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	@echo -e "\033[40m\033[0;32m  Linking" $@ "........\033[0m"
	$(CXX) -o $@ $< $(LDFLAGS)

#------------------------------------------------------------------------------
# Compile object files
#------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: %.cxx
	@echo -e "\033[40m\033[0;34m  Compiling" $< "........\033[0m"
	$(CXX) $(CXXFLAGS) -c $< -o $@

#------------------------------------------------------------------------------
# Generate dependency files
#------------------------------------------------------------------------------
$(DEP_DIR)/%.d: %.cxx
	@echo -e "\033[40m\033[0;33m  Generating dependencies for" $< "........\033[0m"
	@$(CXX) $(CXXFLAGS) -MM $< | sed 's|\(.*\)\.o:|$(OBJ_DIR)/\1.o:|' > $@

# Include dependency files
-include $(DEPENDS)

#------------------------------------------------------------------------------
# Clean targets
#------------------------------------------------------------------------------
clean:
	@echo -e "\033[40m\033[0;34m  Cleaning object files, dependencies, and executables ........\033[0m"
	@rm -rf $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR)
	@echo -e "\033[40m\033[0;34m  Done.\033[0m"

#------------------------------------------------------------------------------
# Help target
#------------------------------------------------------------------------------
help:
	@echo "Available targets:"
	@echo "  all        - Build the executable (default)"
	@echo "  clean      - Remove object files, dependencies, and executables"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Output directories:"
	@echo "  $(OBJ_DIR)/   - Object files (.o)"
	@echo "  $(DEP_DIR)/   - Dependency files (.d)"
	@echo "  $(BIN_DIR)/   - Executables"