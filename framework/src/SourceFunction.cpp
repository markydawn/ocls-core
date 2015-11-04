// TODO: add license and all of that

// we will need to modify ProgramBuilder as well! is it a good idea to incorporate the boundary into the source function?
// control this with boolean values?
// there s definitly room for improvement...

#include <Framework.h>
#include "SourceFunction.h"

namespace ocls {

  SourceFunction::SourceFunction(Domain* domain, Framework* framework, CLSSource::Function* function) :
    CallableFunction(domain, framework, function->parameters), m_program(NULL) {
    ProgramManager* man = framework->getPrograManager(); // TYPO in the code base! TODO: maybe fix it
    ProgramBuilder builder(m_framework->getComputeContext());
    size_t return_values = 0;

    // reflective boundaries for the source function data (ternary is faster right? or is that only in Java?)
    std::string boundary_function = "void reflectBounds(float* S) {\n store(S, fetch_mirror(S, cell.x" 
      + std::string(m_domain.getDimensions() == 1 ? "" : (m_domain.getDimensions() == 2 ? "y" : "yz")) 
      + std::string("), cell.x); \n}");


    //m_program = man->manage(builder.createSourceProgram(&m_domain, function, &return_values)); // WILL NOT COMPILE YET!
    // temporary solution for testing
    m_program = man->manage(builder.createFluxProgram(&m_domain, function, &return_values));

    // boundary this is maybe a bit hacky.
    m_boundary = new BoundaryFunction(&m_domain, framework, framework->getSourceLoader()->loadFromString(boundary_function)->getFunction("reflectBounds"));
    // boundary functions have no return values...

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

    // call the boundary function
    // unsure whether this will work or not... compiles atleast
    m_boundary->call(1, &params[0]); 

    if(m_returnValues.size() == 1) 
      return ReturnType(m_returnValues[0]);
    else
      return Collection::glob(m_returnValues);
  }
}
