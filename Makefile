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

build/demo32.o: tests/eval_test/not_so_easy_test.c
	mkdir -p build
	$(CC) $(CFLAGS) -m32 -c $< -o $@

test: eval_test

# Every suite in one run; `test` stays the school-facing alias for eval_test.
test_all: eval_test edge_case_test lifetime_test

-include $(DEPS)

.PHONY: all clean fclean re edge_case_test lifetime_test eval_test test test_all demo demo32
