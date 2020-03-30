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
#include "AlgorithmImplement.hxx"
#include "Task.hxx"
#include <stdexcept>

namespace WorkloadManager
{
void AlgorithmImplement::addTask(Task* t)
{
  _waitingTasks.push_back(t);
}

bool AlgorithmImplement::empty()const
{
  return _waitingTasks.empty();
}

void AlgorithmImplement::addResource(Resource* r)
{
  // add the resource. The operation is ignored if the resource already exists.
  _resources.emplace(std::piecewise_construct,
                     std::forward_as_tuple(r),
                     std::forward_as_tuple(r)
                    );
}

WorkloadAlgorithm::LaunchInfo AlgorithmImplement::chooseTask()
{
  LaunchInfo result;
  std::list<Task*>::iterator chosenTaskIt;
  for( std::list<Task*>::iterator itTask = _waitingTasks.begin();
      !result.taskFound && itTask != _waitingTasks.end();
      itTask ++)
  {
    const ContainerType* ctype = (*itTask)->type();
    std::map<const Resource *, ResourceLoadInfo>::iterator best_resource;
    best_resource = _resources.end();
    float best_cost = -1.0;
    for(auto itResource = _resources.begin();
        itResource != _resources.end();
        itResource++)
      if(itResource->second.isSupported(ctype))
      {
        if(itResource->second.isAllocPossible(ctype))
        {
          float thisCost = itResource->second.cost(ctype);
          if( best_cost < 0 || best_cost > thisCost)
          {
            best_cost = thisCost;
            best_resource = itResource;
          }
        }
      }
    if(best_resource != _resources.end())
    {
      chosenTaskIt = itTask;
      result.task = (*itTask);
      result.taskFound = true;
      result.worker.resource = best_resource->first;
      result.worker.type = ctype;
      result.worker.index = best_resource->second.alloc(ctype);
    }
  }
  if(result.taskFound)
    _waitingTasks.erase(chosenTaskIt);
  return result;
}

void AlgorithmImplement::liberate(const LaunchInfo& info)
{
  const Resource* r = info.worker.resource;
  unsigned int index = info.worker.index;
  const ContainerType* ctype = info.worker.type;
  std::map<const Resource* ,ResourceLoadInfo>::iterator it = _resources.find(r);
  it->second.free(ctype, index);
}

// ResourceInfoForContainer

AlgorithmImplement::ResourceInfoForContainer::ResourceInfoForContainer
                                (const Resource * r, const ContainerType* ctype)
: _ctype(ctype)
, _resource(r)
, _runningContainers()
, _firstFreeContainer(0)
{
}

unsigned int AlgorithmImplement::ResourceInfoForContainer::maxContainers()const
{
  return float(_resource->nbCores) / _ctype->neededCores;
}

unsigned int  AlgorithmImplement::ResourceInfoForContainer::alloc()
{
  unsigned int result = _firstFreeContainer;
  _runningContainers.insert(result);
  _firstFreeContainer++;
  while(isContainerRunning(_firstFreeContainer))
    _firstFreeContainer++;
  return result;
}

void AlgorithmImplement::ResourceInfoForContainer::free(unsigned int index)
{
  _runningContainers.erase(index);
  if(index < _firstFreeContainer)
    _firstFreeContainer = index;
}

unsigned int AlgorithmImplement::ResourceInfoForContainer::nbRunningContainers()const
{
  return _runningContainers.size();
}

bool AlgorithmImplement::ResourceInfoForContainer::isContainerRunning
                                (unsigned int index)const
{
  return _runningContainers.find(index)!=_runningContainers.end();
}

// ResourceLoadInfo

AlgorithmImplement::ResourceLoadInfo::ResourceLoadInfo(const Resource * r)
: _resource(r)
, _load(0.0)
, _ctypes()
{
}

bool AlgorithmImplement::ResourceLoadInfo::isSupported
                                (const ContainerType* ctype)const
{
  return ctype->neededCores <= _resource->nbCores ;
}
                                          
bool AlgorithmImplement::ResourceLoadInfo::isAllocPossible
                                (const ContainerType* ctype)const
{
  return ctype->neededCores + _load <= _resource->nbCores;
}

float AlgorithmImplement::ResourceLoadInfo::cost
                                (const ContainerType* ctype)const
{
  // TODO: elaborate
  return 1.0;
}

unsigned int AlgorithmImplement::ResourceLoadInfo::alloc
                                (const ContainerType* ctype)
{
  std::map<const ContainerType*, ResourceInfoForContainer>::iterator it;
  it = _ctypes.find(ctype);
  if(it == _ctypes.end())
  {
    // add the type if not found
    it = _ctypes.emplace(std::piecewise_construct,
                         std::forward_as_tuple(ctype),
                         std::forward_as_tuple(_resource, ctype)
                        ).first;
  }
  _load += ctype->neededCores;
  return it->second.alloc();
}

void AlgorithmImplement::ResourceLoadInfo::free
                                (const ContainerType* ctype, int index)
{
  _load -= ctype->neededCores;
  std::map<const ContainerType*, ResourceInfoForContainer>::iterator it;
  it = _ctypes.find(ctype);
  it->second.free(index);
}

}
