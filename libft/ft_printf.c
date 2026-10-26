/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 13:00:08 by donghank          #+#    #+#             */
/*   Updated: 2024/06/13 20:14:04 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "./libft/libft.h"

static int	format_printf(const char fmt, va_list ap)
{
	int	len;

	len = 0;
	if (fmt == 'c')
		len += ft_putchar((char)va_arg(ap, int));
	else if (fmt == 's')
		len += ft_putstr(va_arg(ap, char *));
	else if (fmt == 'd' || fmt == 'i')
		len += ft_putnbr(va_arg(ap, int));
	else if (fmt == 'p')
		len += ft_printf_address(va_arg(ap, void *));
	else if (fmt == 'x')
		len += ft_putnbr_base(va_arg(ap, unsigned int), "0123456789abcdef");
	else if (fmt == 'X')
		len += ft_putnbr_base(va_arg(ap, unsigned int), "0123456789ABCDEF");
	else if (fmt == 'u')
		len += ft_printf_unsigned(va_arg(ap, unsigned int));
	else if (fmt == '%')
	{
		write(1, "%", 1);
		return (1);
	}
	return (len);
}

int	ft_printf(const char *fmt, ...)
{
	va_list	ap;
	int		total_len;
	int		i;

	i = 0;
	total_len = 0;
	va_start(ap, fmt);
	while (fmt[i] != '\0')
	{
		if (fmt[i] == '%')
		{
			total_len += format_printf(fmt[i + 1], ap);
			i++;
		}
		else
			total_len = total_len + ft_putchar(fmt[i]);
		i++;
	}
	va_end(ap);
	return (total_len);
}

