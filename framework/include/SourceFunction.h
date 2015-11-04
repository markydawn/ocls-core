// TODO: add license

// modeled after the coding-style of 

// this file uses Emacs indentation, while the rest uses other indentation. should probably be fixed.

/* Goals for this function class:
 * - Taking points as input (or maybe that is best to do for the data functions. add functionality there for automatically fixing boundary)
 * - "evolve" the points (landslides, avalanches etc.)
 * - Automatically fix ghost cells, so we don't get zeroes at the boundary. No more bottomFix as our experiments have now.
 * - other?
 * 
 */


#ifndef SOURCE_FUNCTION_H
#define SOURCE_FUNCTION_H

// TODO: includes
#include "CLSSource.h"

#include "CallableFunction.h"
#include "BoundaryFunction.h"

namespace ocls {
  /** \ingroup main_api
   *  @{
   */
  class SourceFunction : public CallableFunction {
  public:
    SourceFunction(Domain* domain, Framework* framework, CLSSource::Function* function);
    virtual ~SourceFunction();

  protected:
    /**
     * Launch the source function
     */
    virtual ReturnType call(std::vector<Program::Parameter>& params);

  private:
    Program* m_program;
    CallableFunction* m_boundary;

    std::vector<Data*> m_returnValues;
  };
  /** @} */
}
#endif 
