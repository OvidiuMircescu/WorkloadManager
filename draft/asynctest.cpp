#include <queue>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <list>
#include <future>

class Manager
{
public:
  void addTask();
  void start();
  void stop();
private:
  void runTask(int index);
  void endTask(int index);
  void finishTasks();
private:
  std::map<int, std::future<void> > _tasks;
  std::queue<int> _finishedTasks;
  int _nextIndex = 0;
  std::mutex _data_mutex;
  std::condition_variable _condition;
  bool _stop = false;
  std::list< std::future<void> > _otherThreads;
};

void Manager::addTask()
{
  // if(!_stop)
  std::unique_lock<std::mutex> lock(_data_mutex);
  int currentIndex = _nextIndex++;
  _tasks.emplace(currentIndex, std::async(std::launch::async, [this, currentIndex]
    {
      runTask(currentIndex);
    }));
}

void Manager::runTask(int index)
{
  int t = 1 + std::rand()/((RAND_MAX + 1u)/6);
  std::ostringstream message;
  message  << "hello " << index << " sleep " << t
           << " thread id : " << std::this_thread::get_id()
           <<  std::endl;
  std::cout << message.str();
  std::this_thread::sleep_for(std::chrono::seconds(t));
  endTask(index);
}

void Manager::endTask(int index)
{
  std::unique_lock<std::mutex> lock(_data_mutex);
  _finishedTasks.push(index);
  _condition.notify_one();
 }

void Manager::start()
{
  _otherThreads.emplace_back(std::async(std::launch::async, [this]
    {
      finishTasks();
    }));
}

void Manager::finishTasks()
{
  bool threadStop = false;
  std::cout << "Finishing...." << std::endl;
  while(!threadStop)
  {
    std::cout << "while finish" << std::endl;
    std::unique_lock<std::mutex> lock(_data_mutex);
    _condition.wait(lock, [this] {return !_finishedTasks.empty();});
    while(!_finishedTasks.empty())
    {
      int index = _finishedTasks.front();
      _tasks[index].wait();
      _tasks.erase(index);
      _finishedTasks.pop();
    }
    threadStop = _stop && _tasks.empty();
  }
}

void Manager::stop()
{
  {
  std::unique_lock<std::mutex> lock(_data_mutex);
  _stop = true;
  }
  for(std::future<void>& th : _otherThreads)
    th.wait();
  std::cout << "This is the end!" << std::endl;
}

int main(int argc, char *argv[])
{
  Manager m;
  for(int i=0; i < 10; i++)
    m.addTask();
  //std::this_thread::sleep_for(std::chrono::seconds(1));
  m.start();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  for(int i=0; i < 5; i++)
    m.addTask();
  m.stop();
  return 0;
}
