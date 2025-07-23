#pragma once
class LogUtil
{
public:
	static void error_display(int err_no);
	static void error_display(const char* msg);

	template<typename Args>
	static void PrintLog(const Args& message)
	{
#ifdef _DEBUG
		std::cerr << message << std::endl;
#endif
	}

	template<typename... Args>
	static void PrintLog(std::format_string<Args...> fmt, Args&&... args)
	{
#ifdef _DEBUG
		std::cerr << std::format(fmt, std::forward<Args>(args)...) << std::endl;
#endif
	}
};
