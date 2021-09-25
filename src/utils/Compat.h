#pragma once

#include <functional>
#include <QRunnable>

namespace Utils {
	namespace Compat {
		// Reimplement QRunnable for std::function. Retrocompatability for Qt < 5.15
		class StdFunctionRunnable : public QRunnable {
			std::function<void()> cb;
			public:
				StdFunctionRunnable(std::function<void()> func);
				void run() override;
		};

		QRunnable *CreateFunctionRunnable(std::function<void()> func);
	}
}
