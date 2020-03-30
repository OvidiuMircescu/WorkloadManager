#include <string>
#include <iostream>

#include "../WorkloadManager.hxx"

class MyTask : public WorkloadManager::Task
{
public:
  MyTask():WorkloadManager::Task(), _type()
  {
    _type.id = 5;
    _type.name = "lolo";
    _type.neededCores = 1.0;
  }
  WorkloadManager::ContainerType* type()override {return &_type;}
  void run(const WorkloadManager::Container& c)override 
  {
    std::cout << "Running task on " << c.resource->name << "-"
              << c.type->name << "-" << c.index << std::endl;
  }
  
private:
  WorkloadManager::ContainerType _type;
};

int main(int argc, char *argv[])
{
  WorkloadManager::WorkloadManager wlm;
  WorkloadManager::Resource r;
  r.id = 0;
  r.name = "toto";
  r.nbCores = 42;
  wlm.addResource(r);
  WorkloadManager::ContainerType ct;
  ct.id = 0;
  ct.name = "zozo";
  ct.neededCores = 1.0;
  MyTask tsk;
  wlm.start();
  wlm.addTask(&tsk);
  wlm.addTask(&tsk);
  wlm.addTask(&tsk);
  wlm.stop();
}
