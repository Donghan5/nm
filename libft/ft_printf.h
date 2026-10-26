/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 12:58:00 by donghank          #+#    #+#             */
/*   Updated: 2024/06/09 12:37:44 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <unistd.h>
# include <stdlib.h>
# include <stdarg.h>
# include <stdio.h>
# include <stddef.h>
# include <string.h>
# include <stdint.h>
# include "./libft/libft.h"

int		ft_printf(const char *fmt, ...);
int		ft_putstr(const char *s);
int		ft_putchar(char c);
int		ft_putnbr(int nb);
int		ft_putnbr_base(unsigned int nb, const char *base);
int		ft_printf_address(void *fmt);
int		ft_printf_unsigned(unsigned int nb);
#endif
