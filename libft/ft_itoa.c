/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/20 11:42:37 by donghank          #+#    #+#             */
/*   Updated: 2024/05/24 11:33:37 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	num_len(int n)
{
	size_t	i;

	i = 0;
	if (n < 0)
		n *= -1;
	if (n == 0)
		return (1);
	while (n)
	{
		n /= 10;
		i++;
	}
	return (i);
}

static int	ft_isnegative(int n)
{
	if (n < 0)
		return (1);
	return (0);
}

char	*ft_itoa(int n)
{
	size_t	len;
	char	*ret;
	long	num;
	int		isnegative;

	len = num_len(n);
	isnegative = ft_isnegative(n);
	num = n;
	ret = (char *)malloc(sizeof(char) * (isnegative + len + 1));
	if (!ret)
		return (NULL);
	if (num == 0)
		ret[0] = '0';
	if (num < 0)
	{
		ret[0] = '-';
		num = -num;
	}
	ret[len + isnegative] = '\0';
	while (len-- > 0)
	{
		ret[len + isnegative] = (num % 10) + '0';
		num /= 10;
	}
	return (ret);
}
