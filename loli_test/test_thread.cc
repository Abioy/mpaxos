#include "threadpool.hpp"

using namespace boost::threadpool;

// Some example tasks
void first_task(int x)
{
  sleep(5);
  std::cout << x << std::endl; 
}

void second_task(int y)
{
  std::cout << y << std::endl;
}

void third_task()
{
  std::cout << "HAHA THREE" << std::endl;
}

void execute_with_threadpool()
{
  // Create a thread pool.
  pool tp(2);
  
  // Add some tasks to the pool.
  tp.schedule(boost::bind(first_task, 1));
  tp.schedule(boost::bind(second_task, 2));
  tp.schedule(&third_task);

  tp.wait();

  // Leave this function and wait until all tasks are finished.
}

int main() {
  execute_with_threadpool();
  std::cout << "I'm leaving" << std::endl;
  return 1;
}
