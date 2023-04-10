// Networking
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/URL.hpp>

namespace XYO::Networking::URL {

	String decodeComponent(const String &value) {
		StringReference *out = TMemory<StringReference>::newMemory();
		out->init();
		size_t k;
		for (k = 0; k < value.length(); ++k) {
			if (value[k] == '%') {
				if (k + 2 < value.length()) {
					out->concatenateX(THex<char>::decode(value[k + 1]) << 4 | THex<char>::decode(value[k + 2]));
					k += 2;
				};
				continue;
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
			    (value[k] >= 'A' && value[k] <= 'Z') || (value[k] >= 'a' && value[k] <= 'z') || (value[k] >= '0' && value[k] <= '9') || (value[k] == '_') || (value[k] == '.') || (value[k] == '!') || (value[k] == '~') || (value[k] == '*') || (value[k] == '\'') || (value[k] == '(') || (value[k] == ')')) {
				out->concatenateX(value[k]);
				continue;
			};
			out->concatenateX('%');
			out->concatenateX(THex<char>::encodeUppercase((value[k] >> 4) & 0x0F));
			out->concatenateX(THex<char>::encodeUppercase((value[k]) & 0x0F));
		};
		return out;
	};

	bool getSchemeName(const String &url, String &out) {
		size_t index;
		if (StringCore::indexOf(url, "://", index)) {
			out = url.substring(0, index);
			return true;
		};
		return false;
	};

	bool getHostNameAndPort(const String &url, String &out) {
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (StringCore::indexOf(url, "://", index)) {
			if (StringCore::indexOf(url, "/", index + 3, part)) {
				if (url.substring(index + 3, part - (index + 3)).split2("@", firstPart, secondPart)) {
					out = secondPart;
					return true;
				};
				out = url.substring(index + 3, part - (index + 3));
				return true;
			};
			out = url.substring(index + 3);
			return true;
		};
		return false;
	};

	bool getUsernameAndPassword(const String &url, String &out) {
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (StringCore::indexOf(url, "://", index)) {
			if (StringCore::indexOf(url, "/", index + 3, part)) {
				if (url.substring(index + 3, part - (index + 3)).split2("@", firstPart, secondPart)) {
					out = firstPart;
					return true;
				};
			};
		};
		return false;
	};

	bool getPathAndFileName(const String &url, String &out) {
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (StringCore::indexOf(url, "://", index)) {
			if (StringCore::indexOf(url, "/", index + 3, part)) {
				if (url.substring(part).split2("?", firstPart, secondPart)) {
					out = firstPart;
					return true;
				};
				out = url.substring(part);
				return true;
			};
			out = "/";
			return true;
		};
		return false;
	};

	bool getQuery(const String &url, String &out) {
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		String thirdPart;
		if (url.indexOf("://", 0, index)) {
			if (StringCore::indexOf(url, "/", index + 3, part)) {
				if (url.substring(part + 1).split2("?", firstPart, secondPart)) {
					if (secondPart.split2("#", firstPart, thirdPart)) {
						out = firstPart;
						return true;
					};
				};
			};
		};
		return false;
	};

};
