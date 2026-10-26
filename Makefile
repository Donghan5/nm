NAME = ft_nm
CC = cc
CFLAGS = -Wall -Wextra -Werror
CPPFLAGS = -Iinc -I.

SRCS = src/main.c src/file.c \
	src/elf/elf_ident.c src/elf/elf32.c src/elf/elf64.c \
	src/symbol/symbol_collect.c src/symbol/symbol_type.c \
	src/sort/symbol_sort.c src/print/symbol_print.c src/utils/error.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
LIBFT = libft/libft.a

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $@

$(LIBFT): FORCE
	$(MAKE) -C libft all

$(OBJS): | $(LIBFT)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	$(MAKE) -C libft clean
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(MAKE) -C libft fclean
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

FORCE:

-include $(DEPS)

.PHONY: all clean fclean re test-1 test-1-32 test-2 test-3 test-4 FORCE
