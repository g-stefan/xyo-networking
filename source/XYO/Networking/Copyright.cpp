// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/Copyright.hpp>
#include <XYO/Networking/Copyright.rh>

namespace XYO::Networking::Copyright {

	static const char *copyright_ = XYO_NETWORKING_COPYRIGHT;
	static const char *publisher_ = XYO_NETWORKING_PUBLISHER;
	static const char *company_ = XYO_NETWORKING_COMPANY;
	static const char *contact_ = XYO_NETWORKING_CONTACT;
	static const char *fullCopyright_ = XYO_NETWORKING_FULL_COPYRIGHT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

	const char *fullCopyright() {
		return fullCopyright_;
	};

};
