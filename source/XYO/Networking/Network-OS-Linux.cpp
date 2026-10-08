// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifdef XYO_PLATFORM_OS_LINUX

#	include <sys/resource.h>

#	include <XYO/Networking/Network.hpp>

namespace XYO::Networking::Network {

	class Network_ {
		public:
			bool isValid;

			Network_();
	};

	Network_::Network_() {
		isValid = true;

		// Default soft limit of open files is usually 1024, raise it
		// to the maximum allowed so the process can handle more sockets
		struct rlimit limit;
		if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
			rlim_t maximum = limit.rlim_max;
			if (maximum == RLIM_INFINITY) {
				// Kernel does not accept more than fs.nr_open
				maximum = 1024 * 1024;
				FILE *file = fopen("/proc/sys/fs/nr_open", "rb");
				if (file) {
					unsigned long long nrOpen;
					if (fscanf(file, "%llu", &nrOpen) == 1) {
						maximum = static_cast<rlim_t>(nrOpen);
					};
					fclose(file);
				};
			};
			if (limit.rlim_cur < maximum) {
				limit.rlim_cur = maximum;
				setrlimit(RLIMIT_NOFILE, &limit);
			};
		};
	};

	bool isValid() {
		return (TSingletonProcess<Network_>::getValue())->isValid;
	};

};

#endif
