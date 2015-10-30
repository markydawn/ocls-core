// TODO: add license and all of that

// we will need to modify ProgramBuilder as well! is it a good idea to incorporate the boundary into the source function?
// control this with boolean values?

#include <Framework.h>
#include "SourceFunction.h"

namespace ocls {

  SourceFunction::SourceFunction(Domain* domain, Framework* framework, CLSSource::Function* function) :
    CallableFunction(domain, framework, function->parameters), m_program(NULL) {
    ProgramManager* man = framework->getProgramManager();
    ProgramBuilder builder(m_framework->getComputeContext());
    size_t return_values = 0;
    m_program = man->manage(builder.createSourceProgram(&m_domain, function, &return_values)); // WILL NOT COMPILE YET!

    for(int i = 0; i < return_values; ++i) {
      m_returnValues.push_back(framework->createData(*domain, ""));
    }
  }

  SourceFunction::~SourceFunction() {
    // do cleanup. or are all our resources cleaning up themselves?
  }

  ReturnType SourceFunction::call(std::vector<Program::Parameter> &params) {
    for(int i = 0; i < m_returnValues.size(); ++i) {
      params.push_back(m_returnValues[i]->getParameter());
    }
    ProgramLauncher::launch(m_program, params, false);

    if(m_returnValues.size() == 1) 
      return ReturnType(m_returnValues[0]);
    else
      return Collection::glob(m_returnValues);
  }
}
