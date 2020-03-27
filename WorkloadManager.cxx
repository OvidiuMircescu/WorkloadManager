// Copyright (C) 2020  CEA/DEN, EDF R&D
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
#include "WorkloadManager.hxx"
#include "Task.hxx"
//#include "Container.hxx"

namespace WorkLoadManager
{
  WorkloadManager::~WorkloadManager()
  {
    stop();
    for(Resource* r : _resources)
      delete r;
  }
  
  void WorkloadManager::addResource(Resource r)
  {
    std::unique_lock<std::mutex> lock(_data_mutex);
    _resources.push_back(new Resource(r));
    _startCondition.notify_one();
  }
  
  void WorkloadManager::addTask(Task* t)
  {
    std::unique_lock<std::mutex> lock(_data_mutex);
    _submitedTasks.push_back(t);
    _startCondition.notify_one();
  }

  void WorkloadManager::start()
  {
    _otherThreads.emplace_back(std::async([this]
      {
        runTasks();
      }));
    _otherThreads.emplace_back(std::async([this]
      {
        endTasks();
      }));
  }

  void WorkloadManager::stop()
  {
    {
      std::unique_lock<std::mutex> lock(_data_mutex);
      _stop = true;
    }
    _startCondition.notify_one();
    _endCondition.notify_one();
    for(std::future<void>& th : _otherThreads)
      th.wait();
  }

  void WorkloadManager::runTasks()
  {
    bool threadStop = false;
    while(!threadStop)
    {
      std::unique_lock<std::mutex> lock(_data_mutex);
      _startCondition.wait(lock, [this] {return !_submitedTasks.empty();});
      RunningInfo taskInfo;
      while(chooseTaskToRun(taskInfo))
      {
        _runningTasks.emplace(taskInfo.id, std::async([this, taskInfo]
          {
            runOneTask(taskInfo);
          }));
      }
      threadStop = _stop && _submitedTasks.empty();
    }
  }

  void WorkloadManager::runOneTask(const RunningInfo& taskInfo)
  {
    taskInfo.task->run(taskInfo.worker);

    {
      std::unique_lock<std::mutex> lock(_data_mutex);
      _finishedTasks.push(taskInfo);
      _endCondition.notify_one();
    }
  }

  void WorkloadManager::endTasks()
  {
    bool threadStop = false;
    while(!threadStop)
    {
      std::unique_lock<std::mutex> lock(_data_mutex);
      _endCondition.wait(lock, [this] {return !_finishedTasks.empty();});
      while(!_finishedTasks.empty())
      {
        RunningInfo taskInfo = _finishedTasks.front();
        _runningTasks[taskInfo.id].wait();
        _runningTasks.erase(taskInfo.id);
        _finishedTasks.pop();
        liberate(taskInfo);
      }
      threadStop = _stop && _runningTasks.empty() && _submitedTasks.empty();
      _startCondition.notify_one();
    }
  }

  bool WorkloadManager::chooseTaskToRun(RunningInfo& taskInfo)
  {
    // We are already under the lock
    Task* chosenTask = nullptr;
    if(!_submitedTasks.empty() && !_resources.empty())
    {
      // naive implementation
      // TODO: effective implementation
      chosenTask = _submitedTasks.front();
      _submitedTasks.pop_front();
      TaskId currentIndex = _nextIndex++;
      taskInfo.id = currentIndex;
      taskInfo.task = chosenTask;
      taskInfo.worker.type = chosenTask->type();
      taskInfo.worker.resource = _resources.front();
      taskInfo.worker.index = currentIndex; //bidon
    }
    return chosenTask != nullptr; // no task can be run
  }

  void WorkloadManager::liberate(const RunningInfo& taskInfo)
  {
    
  }
}
