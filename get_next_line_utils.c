/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: grg <grg@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:28:55 by grg               #+#    #+#             */
/*   Updated: 2026/10/18 08:09:39 by grg              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

size_t	ft_strlen(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

char	*ft_strchr(const char *s, int c)
{
	int	i;

	i = 0;
	if (!s)
		return (NULL);
	while (s[i] && s[i] != c)
		i++;
	if (s[i] == (char)c)
		return ((char *)&s[i]);
	return (NULL);
}

char	*ft_strjoin(char *s1, const char *s2)
{
	int		s1_len;
	int		s2_len;
	char	*joined_s;
	int		i;

	if (!s2)
		return (NULL);
	s1_len = ft_strlen(s1);
	s2_len = ft_strlen(s2);
	joined_s = (char *)malloc(s1_len + s2_len + sizeof(char));
	if (!joined_s)
		return (NULL);
	i = -1;
	while (s1[++i])
		joined_s[i] = s1[i];
	i = -1;
	while (s2[++i])
		joined_s[i + s1_len] = s2[i];
	joined_s[s1_len + s2_len] = '\0';
	free(s1);
	return (joined_s);
}
