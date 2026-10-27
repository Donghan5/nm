NAME = ft_nm
CC = cc
CFLAGS = -Wall -Wextra -Werror
CPPFLAGS = -Iinc -I.

SRCS = src/main.c src/file.c \
	src/elf/elf_ident.c src/elf/elf32.c src/elf/elf64.c \
	src/symbol/symbol_collect.c src/symbol/symbol_validate.c src/symbol/symbol_type.c \
	src/sort/sort_symbols.c src/print/print_symbols.c src/utils/error.c
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

fclean: clean
	$(RM) $(NAME)

re: fclean
	$(MAKE) all

test-1:
	CC="$(CC)" bash tests/day1/test_day1.sh

test-1-32:
	CC="$(CC)" bash tests/day1/test_day1.sh 32

test-2:
	CC="$(CC)" bash tests/day2/test_day2.sh

test-3:
	CC="$(CC)" bash tests/day3/test_day3.sh

test-4:
	CC="$(CC)" DAY4_NM="$(DAY4_NM)" bash tests/day4/test_day4.sh

-include $(DEPS)

.PHONY: all clean fclean re test-1 test-1-32 test-2 test-3 test-4
