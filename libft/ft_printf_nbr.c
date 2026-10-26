/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf_nbr.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/02 12:56:25 by donghank          #+#    #+#             */
/*   Updated: 2024/06/12 10:52:32 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "./libft/libft.h"

int	ft_putnbr(int nb)
{
	char	*nbr;
	int		len;

	nbr = ft_itoa(nb);
	if (!nbr)
		return (0);
	len = (int)ft_strlen(nbr);
	ft_putstr(nbr);
	free(nbr);
	return (len);
}

int	ft_putnbr_base(unsigned int nb, const char *base)
{
	int			base_len;
	int			len;
	long long	nbr;

	len = 0;
	nbr = (long long)nb;
	base_len = (int)ft_strlen(base);
	if (base_len < 2 || base_len > 16)
		return (-1);
	if (nbr >= base_len)
		len += ft_putnbr_base((nbr / base_len), base);
	len += ft_putchar(base[nbr % base_len]);
	return (len);
}

int	ft_printf_unsigned(unsigned int nb)
{
	return (ft_putnbr_base(nb, "0123456789"));
}
