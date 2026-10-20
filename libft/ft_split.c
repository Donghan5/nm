/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: donghank <donghank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 10:14:48 by donghank          #+#    #+#             */
/*   Updated: 2024/05/23 14:48:22 by donghank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	word_count(char const *s, char c)
{
	size_t	cnt;
	int		in_word;

	cnt = 0;
	in_word = 0;
	while (*s != '\0')
	{
		if (*s == c)
			in_word = 0;
		else if (*s != c && !in_word)
		{
			in_word = 1;
			cnt++;
		}
		s++;
	}
	return (cnt);
}

static void	free_strs(char **strs)
{
	int	i;

	i = 0;
	while (strs[i] != 0)
	{
		free(strs[i]);
		i++;
	}
	free(strs);
}

static char	*ft_strndup(char const *s, size_t n)
{
	size_t	i;
	char	*new_s;

	i = 0;
	new_s = (char *)malloc(sizeof(char) * (n + 1));
	if (!new_s)
		return (NULL);
	while (i < n && s[i] != '\0')
	{
		new_s[i] = s[i];
		i++;
	}
	new_s[i] = '\0';
	return (new_s);
}

static char	**make_split(char const *s, char c, char **strs, size_t cnt)
{
	size_t	len;
	size_t	i;

	i = 0;
	while (*s != '\0' && i < cnt)
	{
		while (*s == c)
			s++;
		len = 0;
		while (s[len] != '\0' && s[len] != c)
			len++;
		if (len > 0)
		{
			strs[i] = ft_strndup(s, len);
			if (!strs[i])
			{
				free_strs(strs);
				return (NULL);
			}
			i++;
		}
		s += len;
	}
	strs[i] = 0;
	return (strs);
}

char	**ft_split(char const *s, char c)
{
	char	**strs;
	size_t	cnt;

	if (!s)
		return (NULL);
	cnt = word_count(s, c);
	strs = (char **)malloc(sizeof(char *) * (cnt + 1));
	if (!strs)
		return (NULL);
	strs = make_split(s, c, strs, cnt);
	return (strs);
}
