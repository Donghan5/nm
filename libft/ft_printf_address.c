/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf_address.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/02 12:39:37 by donghank          #+#    #+#             */
/*   Updated: 2024/06/12 10:55:45 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "./libft/libft.h"

static void	ptr_address(uintptr_t ptr, int *len)
{
	if (ptr >= 16)
	{
		ptr_address(ptr / 16, len);
		ptr_address(ptr % 16, len);
	}
	else
	{
		if (ptr < 10)
			*len += ft_putchar(ptr + '0');
		else
			*len += ft_putchar(ptr - 10 + 'a');
	}
}

int	ft_printf_address(void *fmt)
{
	uintptr_t	ptr;
	int			len;

	len = 0;
	ptr = (uintptr_t)fmt;
	if (ptr == 0)
		len += ft_putstr("(nil)");
	else
	{
		len += ft_putstr("0x");
		ptr_address(ptr, &len);
	}
	return (len);
}
