/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/20 11:41:46 by donghank          #+#    #+#             */
/*   Updated: 2024/05/23 14:19:31 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	size_t	len;
	char	*start;
	char	*end;
	char	*res;

	if (!s1)
		return (NULL);
	if (!set)
		return (ft_strdup(s1));
	start = (char *)s1;
	while (*start && ft_strchr(set, *start))
		start++;
	if (*start == '\0')
		return (ft_strdup(""));
	len = ft_strlen(s1);
	end = (char *)(s1 + len - 1);
	while (end > s1 && ft_strchr(set, *end))
		end--;
	len = end - start + 1;
	res = (char *)malloc(sizeof(char) * (len + 1));
	if (!res)
		return (NULL);
	ft_strlcpy(res, start, len + 1);
	return (res);
}
