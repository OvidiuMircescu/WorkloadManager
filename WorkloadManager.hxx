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
#ifndef WORKLOADMANAGER_H
#define WORKLOADMANAGER_H
#include <condition_variable> // notifications
#include <thread>

namespace WorkLoadManager
{
  class Task;
  class Resource;
  class Container;
  /**
  * @todo write docs
  */
  class WorkloadManager
  {
  public:
    void addTask(Task* t);
    void addResource(Resource* r);
    void start(); //! start execution thread
    void stop(); //! stop execution thread
  private:
    struct TaskId
    {
      unsigned int id;
      Task* task;
    };
    
    struct Worker
    {
      Container* container;
      std::thread thread;
    };
    
    void endTask(TaskId tid);
    void runOneTask(TaskId tid, Container* c);
    void runTasks();
    bool chooseTaskToRun(TaskId& t, Container*& c);
    void notifyRun(); //! notify the execution thread
    
    bool stop_running=true;
  };
}
#endif // WORKLOADMANAGER_H
