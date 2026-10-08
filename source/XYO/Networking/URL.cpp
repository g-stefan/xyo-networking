// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/URL.hpp>

namespace XYO::Networking::URL {

	String decodeComponent(const String &value) {
		StringReference *out = TMemory<StringReference>::newMemory();
		out->init();
		size_t k;
		for (k = 0; k < value.length(); ++k) {
			if (value[k] == '%') {
				// Invalid or incomplete escape is kept as is
				if ((k + 2 < value.length()) && THex<char>::isValid(value[k + 1]) && THex<char>::isValid(value[k + 2])) {
					out->concatenateX(THex<char>::decode(value[k + 1]) << 4 | THex<char>::decode(value[k + 2]));
					k += 2;
					continue;
				};
			};
			out->concatenateX(value[k]);
		};
		return out;
	};

	String encodeComponent(const String &value) {
		StringReference *out = TMemory<StringReference>::newMemory();
		out->init();
		size_t k;
		for (k = 0; k < value.length(); ++k) {
			if (
			    (value[k] >= 'A' && value[k] <= 'Z') || (value[k] >= 'a' && value[k] <= 'z') || (value[k] >= '0' && value[k] <= '9') || (value[k] == '-') || (value[k] == '_') || (value[k] == '.') || (value[k] == '!') || (value[k] == '~') || (value[k] == '*') || (value[k] == '\'') || (value[k] == '(') || (value[k] == ')')) {
				out->concatenateX(value[k]);
				continue;
			};
			out->concatenateX('%');
			out->concatenateX(THex<char>::encodeUppercase((value[k] >> 4) & 0x0F));
			out->concatenateX(THex<char>::encodeUppercase((value[k]) & 0x0F));
		};
		return out;
	};

	// Scheme is ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) followed by "://"
	static bool getSchemeLength(const String &url, size_t &length) {
		size_t k;
		char x;
		for (k = 0; k < url.length(); ++k) {
			x = url[k];
			if (((x >= 'A') && (x <= 'Z')) || ((x >= 'a') && (x <= 'z'))) {
				continue;
			};
			if ((k > 0) && (((x >= '0') && (x <= '9')) || (x == '+') || (x == '-') || (x == '.'))) {
				continue;
			};
			break;
		};
		if ((k == 0) || (k + 2 >= url.length())) {
			return false;
		};
		if ((url[k] != ':') || (url[k + 1] != '/') || (url[k + 2] != '/')) {
			return false;
		};
		length = k;
		return true;
	};

	bool getSchemeName(const String &url, String &out) {
		size_t length;
		if (getSchemeLength(url, length)) {
			out = url.substring(0, length);
			return true;
		};
		return false;
	};

	// Authority is the part after "://" up to the first '/', '?' or '#'
	static bool getAuthority(const String &url, size_t &start, size_t &end) {
		if (!getSchemeLength(url, start)) {
			return false;
		};
		start += 3;
		for (end = start; end < url.length(); ++end) {
			if ((url[end] == '/') || (url[end] == '?') || (url[end] == '#')) {
				break;
			};
		};
		return true;
	};

	bool getHostNameAndPort(const String &url, String &out) {
		size_t start;
		size_t end;
		String firstPart;
		String secondPart;
		if (getAuthority(url, start, end)) {
			String authority = url.substring(start, end - start);
			if (authority.split2FromEnd("@", firstPart, secondPart)) {
				out = secondPart;
				return true;
			};
			out = authority;
			return true;
		};
		return false;
	};

	bool getUsernameAndPassword(const String &url, String &out) {
		size_t start;
		size_t end;
		String firstPart;
		String secondPart;
		if (getAuthority(url, start, end)) {
			if (url.substring(start, end - start).split2FromEnd("@", firstPart, secondPart)) {
				out = firstPart;
				return true;
			};
		};
		return false;
	};

	bool getPathAndFileName(const String &url, String &out) {
		size_t start;
		size_t end;
		size_t k;
		if (getAuthority(url, start, end)) {
			for (k = end; k < url.length(); ++k) {
				if ((url[k] == '?') || (url[k] == '#')) {
					break;
				};
			};
			if (k == end) {
				out = "/";
				return true;
			};
			out = url.substring(end, k - end);
			return true;
		};
		return false;
	};

	bool getQuery(const String &url, String &out) {
		size_t start;
		size_t end;
		size_t k;
		if (getAuthority(url, start, end)) {
			for (; end < url.length(); ++end) {
				if (url[end] == '#') {
					return false;
				};
				if (url[end] == '?') {
					for (k = end + 1; k < url.length(); ++k) {
						if (url[k] == '#') {
							break;
						};
					};
					out = url.substring(end + 1, k - (end + 1));
					return true;
				};
			};
		};
		return false;
	};

};
