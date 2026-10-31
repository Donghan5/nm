NAME = ft_nm
CC = cc
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

$(OBJ_DIR)/src/symbol/symbol_tools.o: CPPFLAGS += -include inc/ft_nm.h

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(SRCS:.c=.o) $(SRCS:.c=.d)

fclean: clean
	$(RM) $(NAME)

re: fclean
	$(MAKE) all

days_test:
	CC="$(CC)" python3 tests/days_test/run.py

-include $(DEPS)

.PHONY: all clean fclean re days_test
