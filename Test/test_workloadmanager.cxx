#include <string>
#include <iostream>

#include "../WorkloadManager.hxx"

class MyTask : public WorkLoadManager::Task
{
public:
  MyTask():WorkLoadManager::Task(), _type()
  {
    _type.id = 5;
    _type.name = "lolo";
    _type.neededCores = 1.0;
  }
  WorkLoadManager::ContainerType* type()override {return &_type;}
  void run(const WorkLoadManager::Container& c)override 
  {
    std::cout << "Running task on " << c.resource->name << "-"
              << c.type->name << "-" << c.index << std::endl;
  }
  
private:
  WorkLoadManager::ContainerType _type;
};

int main(int argc, char *argv[])
{
  WorkLoadManager::WorkloadManager wlm;
  WorkLoadManager::Resource r;
  r.id = 0;
  r.name = "toto";
  r.nbCores = 42;
  wlm.addResource(r);
  WorkLoadManager::ContainerType ct;
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
