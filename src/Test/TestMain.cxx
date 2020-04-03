// Copyright (C) 2020  EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestRunner.h>
#include <stdexcept>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <sstream>
#include <vector>

#include <chrono>
#include <ctime>
#include <thread>

#include "../WorkloadManager.hxx"
#include "../AlgorithmImplement.hxx"

//#define DEBUG_LOG

class MyTask;
class AbstractChecker
{
public:
  virtual void check(const WorkloadManager::Container& c, MyTask* t)=0;
};

template <std::size_t size_R, std::size_t size_T>
class Checker : public AbstractChecker
{
public:
  Checker();
  void check(const WorkloadManager::Container& c, MyTask* t)override;
  void globalCheck();

  WorkloadManager::Resource resources[size_R];
  WorkloadManager::ContainerType types[size_T];
private:
  std::mutex _mutex;
  int _maxContainersForResource[size_R][size_T];
};

class MyTask : public WorkloadManager::Task
{
public:
  const WorkloadManager::ContainerType* type()const override {return _type;}
  void run(const WorkloadManager::Container& c)override
  {
    _check->check(c, this);

#ifdef DEBUG_LOG
    std::ostringstream message;
    message << "Running task " << _id << " on " << c.resource->name << "-"
              << c.type->name << "-" << c.index << std::endl;
    std::cerr << message.str();
#endif
    std::this_thread::sleep_for(std::chrono::seconds(_sleep));
#ifdef DEBUG_LOG
    std::ostringstream message2;
    message2 << "Finish task " << _id << std::endl;
    std::cerr << message2.str();
#endif
  }

  void reset(int id,
             const WorkloadManager::ContainerType* type,
             int sleep,
             AbstractChecker * check
            )
  {
    _id = id;
    _type = type;
    _sleep = sleep;
    _check = check;
  }
private:
  int _id = 0;
  const WorkloadManager::ContainerType* _type = nullptr;
  int _sleep = 0;
  AbstractChecker * _check;
};

template <std::size_t size_R, std::size_t size_T>
Checker<size_R, size_T>::Checker()
{
  for(std::size_t i=0; i < size_R; i ++)
  {
    resources[i].id = i;
    std::ostringstream name;
    name << "r" << i;
    resources[i].name = name.str();
  }

  for(std::size_t i=0; i < size_T; i ++)
  {
    types[i].id = i;
    std::ostringstream name;
    name << "t" << i;
    types[i].name = name.str();
  }

  for(std::size_t i=0; i < size_R; i++)
    for(std::size_t j=0; j < size_T; j++)
      _maxContainersForResource[i][j] = 0;
}

template <std::size_t size_R, std::size_t size_T>
void Checker<size_R, size_T>::check(const WorkloadManager::Container& c,
                                    MyTask* t)
{
  std::unique_lock<std::mutex> lock(_mutex);
  int& max = _maxContainersForResource[c.resource->id][c.type->id];
  if( max < c.index)
    max = c.index;
}

template <std::size_t size_R, std::size_t size_T>
void Checker<size_R, size_T>::globalCheck()
{
  for(std::size_t i=0; i < size_R; i++)
  {
    float global_max = 0;
    for(std::size_t j=0; j < size_T; j++)
    {
      int max = _maxContainersForResource[i][j];
#ifdef DEBUG_LOG
      std::cout << resources[i].name << ", " << types[j].name << ":"
                << max+1 << std::endl;
#endif
      CPPUNIT_ASSERT( (max+1) * types[j].neededCores <= resources[i].nbCores );
      global_max += types[j].neededCores * float(max+1);
    }
#ifdef DEBUG_LOG
    std::cout << resources[i].name << " global: " << global_max << std::endl;
#endif
    CPPUNIT_ASSERT(global_max >= resources[i].nbCores); // cores fully used
  }
}


class MyTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyTest);
  CPPUNIT_TEST(atest);
  CPPUNIT_TEST_SUITE_END();
public:
  void atest();
};

void MyTest::atest()
{
  constexpr std::size_t resourcesNumber = 2;
  constexpr std::size_t typesNumber = 2;
  Checker<resourcesNumber, typesNumber> check;
  check.resources[0].nbCores = 10;
  check.resources[1].nbCores = 18;
  check.types[0].neededCores = 4.0;
  check.types[1].neededCores = 1.0;

#ifdef DEBUG_LOG
  std::cout << std::endl;
  for(std::size_t i=0; i < resourcesNumber; i ++)
    std::cout << check.resources[i].name << " has "
              << check.resources[i].nbCores << " cores." << std::endl;
  for(std::size_t i=0; i < typesNumber; i ++)
    std::cout << check.types[i].name << " needs "
              << check.types[i].neededCores << " cores." << std::endl;
#endif

  constexpr std::size_t tasksNumber = 100;
  MyTask tasks[tasksNumber];
  for(std::size_t i = 0; i < tasksNumber / 2; i++)
    tasks[i].reset(i, &check.types[0], 2, &check);
  for(std::size_t i = tasksNumber / 2; i < tasksNumber; i++)
    tasks[i].reset(i, &check.types[1], 1, &check);

#ifdef DEBUG_LOG
  std::cout << "Number of tasks: " << tasksNumber << std::endl;
  std::cout << "Tasks from 0 to " << tasksNumber / 2 << " are " 
            << tasks[0].type()->name << std::endl;
  std::cout << "Tasks from " << tasksNumber / 2 << " to " << tasksNumber
            << " are " << tasks[tasksNumber / 2].type()->name << std::endl;
#endif

  WorkloadManager::AlgorithmImplement algo;
  WorkloadManager::WorkloadManager wlm(algo);
  for(std::size_t i=0; i < resourcesNumber; i ++)
    wlm.addResource(&check.resources[i]);

//   for(std::size_t i = 0; i < tasksNumber; i++)
//     wlm.addTask(&tasks[i]);
  for(std::size_t i = tasksNumber-1; i > 0; i--)
    wlm.addTask(&tasks[i]);
  wlm.addTask(&tasks[0]);

  wlm.start(); // tasks can be added before start.
  wlm.stop();
  check.globalCheck();
}

CPPUNIT_TEST_SUITE_REGISTRATION(MyTest);

// ============================================================================
/*!
 *  Main program source for Unit Tests with cppunit package does not depend
 *  on actual tests, so we use the same for all partial unit tests.
 */
// ============================================================================

int main(int argc, char* argv[])
{
  // --- Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // ---  Add a listener that collects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // ---  Add a listener that print dots as test run.
#ifdef WIN32
  CPPUNIT_NS::TextTestProgressListener progress;
#else
  CPPUNIT_NS::BriefTestProgressListener progress;
#endif
  controller.addListener( &progress );      

  // ---  Get the top level suite from the registry

  CPPUNIT_NS::Test *suite =
    CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  // ---  Adds the test to the list of test to run

  CPPUNIT_NS::TestRunner runner;
  runner.addTest( suite );
  runner.run( controller);

  // ---  Print test in a compiler compatible format.
  std::ofstream testFile;
  testFile.open("test.log", std::ios::out | std::ios::app);
  testFile << "------ Idefix test log:" << std::endl;
  CPPUNIT_NS::CompilerOutputter outputter( &result, testFile );
  outputter.write(); 

  // ---  Run the tests.

  bool wasSucessful = result.wasSuccessful();
  testFile.close();

  // ---  Return error code 1 if the one of test failed.

  return wasSucessful ? 0 : 1;
}
