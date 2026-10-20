/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ubuntu <ubuntu@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/20 11:45:55 by donghank          #+#    #+#             */
/*   Updated: 2024/05/25 07:20:24 by ubuntu           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_memcmp(const void *s1, const void *s2, size_t n)
{
	unsigned const char	*d1;
	unsigned const char	*d2;

	d1 = (unsigned const char *)s1;
	d2 = (unsigned const char *)s2;
	while (n--)
	{
		if (*d1 != *d2)
			return ((int)(*d1 - *d2));
		d1++;
		d2++;
	}
	return (0);
}
