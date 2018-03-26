/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2017 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include "calcurse.h"

struct utf8_range {
	int min, max, width;
};

static const struct utf8_range utf8_widthtab[] = {
	{0x00300, 0x0036f, 0},
	{0x00483, 0x00489, 0},
	{0x00591, 0x005bd, 0},
	{0x005bf, 0x005bf, 0},
	{0x005c1, 0x005c2, 0},
	{0x005c4, 0x005c5, 0},
	{0x005c7, 0x005c7, 0},
	{0x00610, 0x0061a, 0},
	{0x0064b, 0x0065e, 0},
	{0x00670, 0x00670, 0},
	{0x006d6, 0x006dc, 0},
	{0x006de, 0x006e4, 0},
	{0x006e7, 0x006e8, 0},
	{0x006ea, 0x006ed, 0},
	{0x00711, 0x00711, 0},
	{0x00730, 0x0074a, 0},
	{0x007a6, 0x007b0, 0},
	{0x007eb, 0x007f3, 0},
	{0x00816, 0x00819, 0},
	{0x0081b, 0x00823, 0},
	{0x00825, 0x00827, 0},
	{0x00829, 0x0082d, 0},
	{0x00900, 0x00903, 0},
	{0x0093c, 0x0093c, 0},
	{0x0093e, 0x0094e, 0},
	{0x00951, 0x00955, 0},
	{0x00962, 0x00963, 0},
	{0x00981, 0x00983, 0},
	{0x009bc, 0x009bc, 0},
	{0x009be, 0x009c4, 0},
	{0x009c7, 0x009c8, 0},
	{0x009cb, 0x009cd, 0},
	{0x009d7, 0x009d7, 0},
	{0x009e2, 0x009e3, 0},
	{0x00a01, 0x00a03, 0},
	{0x00a3c, 0x00a3c, 0},
	{0x00a3e, 0x00a42, 0},
	{0x00a47, 0x00a48, 0},
	{0x00a4b, 0x00a4d, 0},
	{0x00a51, 0x00a51, 0},
	{0x00a70, 0x00a71, 0},
	{0x00a75, 0x00a75, 0},
	{0x00a81, 0x00a83, 0},
	{0x00abc, 0x00abc, 0},
	{0x00abe, 0x00ac5, 0},
	{0x00ac7, 0x00ac9, 0},
	{0x00acb, 0x00acd, 0},
	{0x00ae2, 0x00ae3, 0},
	{0x00b01, 0x00b03, 0},
	{0x00b3c, 0x00b3c, 0},
	{0x00b3e, 0x00b44, 0},
	{0x00b47, 0x00b48, 0},
	{0x00b4b, 0x00b4d, 0},
	{0x00b56, 0x00b57, 0},
	{0x00b62, 0x00b63, 0},
	{0x00b82, 0x00b82, 0},
	{0x00bbe, 0x00bc2, 0},
	{0x00bc6, 0x00bc8, 0},
	{0x00bca, 0x00bcd, 0},
	{0x00bd7, 0x00bd7, 0},
	{0x00c01, 0x00c03, 0},
	{0x00c3e, 0x00c44, 0},
	{0x00c46, 0x00c48, 0},
	{0x00c4a, 0x00c4d, 0},
	{0x00c55, 0x00c56, 0},
	{0x00c62, 0x00c63, 0},
	{0x00c82, 0x00c83, 0},
	{0x00cbc, 0x00cbc, 0},
	{0x00cbe, 0x00cc4, 0},
	{0x00cc6, 0x00cc8, 0},
	{0x00cca, 0x00ccd, 0},
	{0x00cd5, 0x00cd6, 0},
	{0x00ce2, 0x00ce3, 0},
	{0x00d02, 0x00d03, 0},
	{0x00d3e, 0x00d44, 0},
	{0x00d46, 0x00d48, 0},
	{0x00d4a, 0x00d4d, 0},
	{0x00d57, 0x00d57, 0},
	{0x00d62, 0x00d63, 0},
	{0x00d82, 0x00d83, 0},
	{0x00dca, 0x00dca, 0},
	{0x00dcf, 0x00dd4, 0},
	{0x00dd6, 0x00dd6, 0},
	{0x00dd8, 0x00ddf, 0},
	{0x00df2, 0x00df3, 0},
	{0x00e31, 0x00e31, 0},
	{0x00e34, 0x00e3a, 0},
	{0x00e47, 0x00e4e, 0},
	{0x00eb1, 0x00eb1, 0},
	{0x00eb4, 0x00eb9, 0},
	{0x00ebb, 0x00ebc, 0},
	{0x00ec8, 0x00ecd, 0},
	{0x00f18, 0x00f19, 0},
	{0x00f35, 0x00f35, 0},
	{0x00f37, 0x00f37, 0},
	{0x00f39, 0x00f39, 0},
	{0x00f3e, 0x00f3f, 0},
	{0x00f71, 0x00f84, 0},
	{0x00f86, 0x00f87, 0},
	{0x00f90, 0x00f97, 0},
	{0x00f99, 0x00fbc, 0},
	{0x00fc6, 0x00fc6, 0},
	{0x0102b, 0x0103e, 0},
	{0x01056, 0x01059, 0},
	{0x0105e, 0x01060, 0},
	{0x01062, 0x01064, 0},
	{0x01067, 0x0106d, 0},
	{0x01071, 0x01074, 0},
	{0x01082, 0x0108d, 0},
	{0x0108f, 0x0108f, 0},
	{0x0109a, 0x0109d, 0},
	{0x01100, 0x0115f, 2},
	{0x011a3, 0x011a7, 2},
	{0x011fa, 0x011ff, 2},
	{0x0135f, 0x0135f, 0},
	{0x01712, 0x01714, 0},
	{0x01732, 0x01734, 0},
	{0x01752, 0x01753, 0},
	{0x01772, 0x01773, 0},
	{0x017b6, 0x017d3, 0},
	{0x017dd, 0x017dd, 0},
	{0x0180b, 0x0180d, 0},
	{0x018a9, 0x018a9, 0},
	{0x01920, 0x0192b, 0},
	{0x01930, 0x0193b, 0},
	{0x019b0, 0x019c0, 0},
	{0x019c8, 0x019c9, 0},
	{0x01a17, 0x01a1b, 0},
	{0x01a55, 0x01a5e, 0},
	{0x01a60, 0x01a7c, 0},
	{0x01a7f, 0x01a7f, 0},
	{0x01b00, 0x01b04, 0},
	{0x01b34, 0x01b44, 0},
	{0x01b6b, 0x01b73, 0},
	{0x01b80, 0x01b82, 0},
	{0x01ba1, 0x01baa, 0},
	{0x01c24, 0x01c37, 0},
	{0x01cd0, 0x01cd2, 0},
	{0x01cd4, 0x01ce8, 0},
	{0x01ced, 0x01ced, 0},
	{0x01cf2, 0x01cf2, 0},
	{0x01dc0, 0x01de6, 0},
	{0x01dfd, 0x01dff, 0},
	{0x020d0, 0x020f0, 0},
	{0x02329, 0x0232a, 2},
	{0x02cef, 0x02cf1, 0},
	{0x02de0, 0x02dff, 0},
	{0x02e80, 0x02e99, 2},
	{0x02e9b, 0x02ef3, 2},
	{0x02f00, 0x02fd5, 2},
	{0x02ff0, 0x02ffb, 2},
	{0x03000, 0x03029, 2},
	{0x0302a, 0x0302f, 0},
	{0x03030, 0x0303e, 2},
	{0x03041, 0x03096, 2},
	{0x03099, 0x0309a, 0},
	{0x0309b, 0x030ff, 2},
	{0x03105, 0x0312d, 2},
	{0x03131, 0x0318e, 2},
	{0x03190, 0x031b7, 2},
	{0x031c0, 0x031e3, 2},
	{0x031f0, 0x0321e, 2},
	{0x03220, 0x03247, 2},
	{0x03250, 0x032fe, 2},
	{0x03300, 0x04dbf, 2},
	{0x04e00, 0x0a48c, 2},
	{0x0a490, 0x0a4c6, 2},
	{0x0a66f, 0x0a672, 0},
	{0x0a67c, 0x0a67d, 0},
	{0x0a6f0, 0x0a6f1, 0},
	{0x0a802, 0x0a802, 0},
	{0x0a806, 0x0a806, 0},
	{0x0a80b, 0x0a80b, 0},
	{0x0a823, 0x0a827, 0},
	{0x0a880, 0x0a881, 0},
	{0x0a8b4, 0x0a8c4, 0},
	{0x0a8e0, 0x0a8f1, 0},
	{0x0a926, 0x0a92d, 0},
	{0x0a947, 0x0a953, 0},
	{0x0a960, 0x0a97c, 2},
	{0x0a980, 0x0a983, 0},
	{0x0a9b3, 0x0a9c0, 0},
	{0x0aa29, 0x0aa36, 0},
	{0x0aa43, 0x0aa43, 0},
	{0x0aa4c, 0x0aa4d, 0},
	{0x0aa7b, 0x0aa7b, 0},
	{0x0aab0, 0x0aab0, 0},
	{0x0aab2, 0x0aab4, 0},
	{0x0aab7, 0x0aab8, 0},
	{0x0aabe, 0x0aabf, 0},
	{0x0aac1, 0x0aac1, 0},
	{0x0abe3, 0x0abea, 0},
	{0x0abec, 0x0abed, 0},
	{0x0ac00, 0x0d7a3, 2},
	{0x0d7b0, 0x0d7c6, 2},
	{0x0d7cb, 0x0d7fb, 2},
	{0x0f900, 0x0faff, 2},
	{0x0fb1e, 0x0fb1e, 0},
	{0x0fe00, 0x0fe0f, 0},
	{0x0fe10, 0x0fe19, 2},
	{0x0fe20, 0x0fe26, 0},
	{0x0fe30, 0x0fe52, 2},
	{0x0fe54, 0x0fe66, 2},
	{0x0fe68, 0x0fe6b, 2},
	{0x0ff01, 0x0ff60, 2},
	{0x0ffe0, 0x0ffe6, 2},
	{0x101fd, 0x101fd, 0},
	{0x10a01, 0x10a03, 0},
	{0x10a05, 0x10a06, 0},
	{0x10a0c, 0x10a0f, 0},
	{0x10a38, 0x10a3a, 0},
	{0x10a3f, 0x10a3f, 0},
	{0x11080, 0x11082, 0},
	{0x110b0, 0x110ba, 0},
	{0x1d165, 0x1d169, 0},
	{0x1d16d, 0x1d172, 0},
	{0x1d17b, 0x1d182, 0},
	{0x1d185, 0x1d18b, 0},
	{0x1d1aa, 0x1d1ad, 0},
	{0x1d242, 0x1d244, 0},
	{0x1f200, 0x1f200, 2},
	{0x1f210, 0x1f231, 2},
	{0x1f240, 0x1f248, 2},
	{0x20000, 0x2fffd, 2},
	{0x30000, 0x3fffd, 2},
	{0xe0100, 0xe01ef, 0}
};

/* Decode a UTF-8 encoded character. Return the Unicode code point. */
int utf8_decode(const char *s)
{
	if (UTF8_ISCONT(*s))
		return -1;

	switch (UTF8_LENGTH(*s)) {
	case 1:
		return s[0];
	case 2:
		return (s[1] & 0x3f) | (s[0] & 0x1f) << 6;
	case 3:
		return ((s[2] & 0x3f) | (s[1] & 0x3f) << 6) |
			(s[0] & 0x0f) << 12;
	case 4:
		return (((s[3] & 0x3f) | (s[2] & 0x3f) << 6) |
			(s[1] & 0x3f) << 12) | (s[0] & 0x7) << 18;
	default:
		return -1;
	}
}

/*
 * Encode a Unicode code point.
 * Return a pointer to the resulting UTF-8 encoded character.
 */
char *utf8_encode(int u)
{
	static char c[5]; /* 4 bytes + string termination */

	/* 0x0000 - 0x007F: 0xxxxxxx */
	if (u < 0x80) {
		*(c + 1) = '\0';
		*c = u;
	/* 0x0080 - 0x07FF: 110xxxxx 10xxxxxx */
	} else if (u < 0x800) {
		*(c + 2) = '\0';
		*(c + 1) = (u      & 0x3F) | 0x80;
		*c       = (u >> 6)        | 0xC0;
	/* 0x0800 - 0xFFFF: 1110xxxx 10xxxxxx 10xxxxxx */
	} else if (u < 0x10000) {
		*(c + 3) = '\0';
		*(c + 2) = (u      & 0x3F) | 0x80;
		*(c + 1) = (u >> 6 & 0x3F) | 0x80;
		*c       = (u >> 12)       | 0xE0;
	} else if (u < 0x110000) {
	/* 0x10000 - 0x10FFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		*(c + 4) = '\0';
		*(c + 3) = (u      & 0x3F) | 0x80;
		*(c + 2) = (u >> 6 & 0x3F) | 0x80;
		*(c + 1) = (u >> 12& 0x3F) | 0x80;
		*c       = (u >> 18)       | 0xF0;
	} else
		return NULL;
	return c;
}

/* Get the display width of a UTF-8 character. */
int utf8_width(char *s)
{
	int val, low, high, cur;

	if (UTF8_ISCONT(*s))
		return 0;
	val = utf8_decode(s);
	low = 0;
	high = ARRAY_SIZE(utf8_widthtab);
	do {
		cur = (low + high) / 2;
		if (val >= utf8_widthtab[cur].min) {
			if (val <= utf8_widthtab[cur].max)
				return utf8_widthtab[cur].width;
			else
				low = cur + 1;
		} else {
			high = cur - 1;
		}
	}
	while (low <= high);

	return 1;
}

/* Get the width of a UTF-8 string. */
int utf8_strwidth(char *s)
{
	int width = 0;

	for (; *s; s++)
		width += utf8_width(s);
	return width;
}

/* Trim a UTF-8 string if it is too long, possibly adding dots for padding. */
int utf8_chop(char *s, int width)
{
	int i, n = 0;

	for (i = 0; s[i] && width > 0; i++) {
		if (!UTF8_ISCONT(s[i]))
			width -= utf8_width(&s[i]);
		if (width >= 3)
			n = i + 1;
	}

	if (s[i] == '\0')
		return 0;

	if (s[n] != '\0' && s[n + 1] != '\0' && s[n + 2] != '\0') {
		s[n] = s[n + 1] = s[n + 2] = '.';
		s[n + 3] = '\0';
	}

	return 1;
}
