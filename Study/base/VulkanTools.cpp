//#include "pch.h"
//#include <iostream>
#include <fstream>
#include <string>
//#include <iosfwd>
//#include <cstring>
#include "VulkanTools.h"
namespace mg
{
	/*
	namespace tools
	{
		bool errorModeSilent = false;

		void exitFatal(const std::string& message, int32_t exitCode)
		{
#if defined(_WIN32)
			if (!errorModeSilent) {
				MessageBox(NULL, message.c_str(), NULL, MB_OK | MB_ICONERROR);
			}
#elif defined(__ANDROID__)
			LOGE("Fatal error: %s", message.c_str());
			vks::android::showAlert(message.c_str());
#endif
			std::cerr << message << "\n";
#if !defined(__ANDROID__)
			exit(exitCode);
#endif
		}

		void exitFatal(const std::string& message, VkResult resultCode)
		{
			exitFatal(message, (int32_t)resultCode);
		}
		bool fileExists(const std::string& filename) {
			std::ifstream f(filename.c_str());
			return !f.fail();
		}
	}
	*/
}
