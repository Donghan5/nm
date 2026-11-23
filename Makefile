NAME = ft_nm
CC ?= cc
CFLAGS = -Wall -Wextra -Werror
CPPFLAGS = -Iinc -I.

SRCS = src/main.c src/utils/file.c src/utils/error.c \
	src/elf/elf_ident.c src/elf/elf32.c src/elf/elf64.c src/elf/get_elf_section.c \
	src/symbol/symbol_collect_32.c src/symbol/symbol_collect_64.c \
	src/symbol/symbol_validate.c src/symbol/symbol_type.c \
	src/symbol/symbol_tools.c src/symbol/symbol_sort.c src/symbol/symbol_print.c
OBJ_DIR = build/obj
DEP_DIR = build/dep
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
DEPS = $(SRCS:%.c=$(DEP_DIR)/%.d)

all: $(NAME)

help:
	@echo "make [all]          Build $(NAME)"
	@echo "make $(NAME)          Build the $(NAME) executable"
	@echo "make clean          Remove object and dependency files"
	@echo "make fclean         Remove build files and the executable"
	@echo "make re             Clean and rebuild"
	@echo "make test           Run the evaluation tests"
	@echo "make eval_test      Run the evaluation tests"
	@echo "make edge_case_test Run the edge-case tests"
	@echo "make lifetime_test  Run the lifetime tests"
	@echo "make test_all       Run all test suites"
	@echo "make valgrind       Check evaluation fixtures with Valgrind"
	@echo "make demo           Run the demo"
	@echo "make demo32         Compare symbol output for a 32-bit object"
	@echo "make proof          Run the proof script"
	@echo "make help           Show available commands"

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@) $(dir $(DEP_DIR)/$*.d)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(SRCS:.c=.o) $(SRCS:.c=.d)
	$(RM) build/demo32.o

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(OBJ_DIR) $(DEP_DIR) build

re: fclean
	$(MAKE) all

edge_case_test:
	CC="$(CC)" bash tests/edge_case_test/run.sh

lifetime_test:
	CC="$(CC)" bash tests/edge_case_test/lifetime/run.sh

eval_test:
	CC="$(CC)" bash tests/eval_test/run.sh

demo: $(NAME)
	CC="$(CC)" bash tests/demo.sh

demo32: $(NAME) build/demo32.o
	file build/demo32.o
	@echo "GNU nm:"
	LC_ALL=C nm build/demo32.o
	@echo "ft_nm:"
	LC_ALL=C ./$(NAME) build/demo32.o

build/demo32.o: tests/eval_test/symbols.s
	mkdir -p build
	$(AS) --32 $< -o $@

valgrind: export CCACHE_DISABLE ?= 1
valgrind: $(NAME) build/demo32.o
	bash tests/eval_test/valgrind.sh ./$(NAME) build/demo32.o

proof: $(NAME)
	bash tests/proof.sh

test: eval_test

# Every suite in one run; `test` stays the school-facing alias for eval_test.
test_all: eval_test edge_case_test lifetime_test

-include $(DEPS)

.PHONY: all help clean fclean re edge_case_test lifetime_test eval_test test test_all valgrind proof demo demo32
