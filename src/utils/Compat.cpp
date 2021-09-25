#include "Compat.h"

Utils::Compat::StdFunctionRunnable::StdFunctionRunnable(std::function<void()> func) :
	cb(std::move(func))
{
}

void Utils::Compat::StdFunctionRunnable::run()
{
	cb();
}

QRunnable *Utils::Compat::CreateFunctionRunnable(std::function<void()> func)
{
	return new Utils::Compat::StdFunctionRunnable(std::move(func));
}
