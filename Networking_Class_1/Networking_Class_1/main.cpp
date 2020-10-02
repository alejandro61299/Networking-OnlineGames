#include <stdio.h>
#include <stdlib.h>
#include <thread>   // <-- Inlcude this for threading
#include <mutex>    // <-- For critical sections

long long int myGlobal = 0;
std::mutex mtx;


void Function()
{
	for (long long int i = 0; i < 1000000; ++i)
	{
		std::unique_lock<std::mutex> lock(mtx);  // <--  Block the memory accces for other threads
		++myGlobal;
		// <--  Here the mutex variable (mtx) disapears
	}

	printf("I added 1000000 to globak!\n");
}

int main()
{
	std::thread t[2] = { std::thread(Function), std::thread(Function) };  // <-- Start the thread
	printf("Hola desde el main thread\n");
	t[0].join();  // <-- Main waits thread finalization
	t[1].join();
	system("pause");
	return 0;
}
