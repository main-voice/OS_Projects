#include "Semaphore.h"

void Semaphore::wait()
{
	std::unique_lock<std::mutex> lock(mtx);
	if (--count < 0) {
		cv.wait(lock); // 为了克服忙等待占用CPU资源，该进程阻塞自己，将该进程放在一个与信号量相关的等待队列中。CPU会执行另一个进程
	}
}

void Semaphore::signal()
{
	std::unique_lock<std::mutex> lock(mtx);
	if (++count <= 0) {
		cv.notify_one();
	}
}
