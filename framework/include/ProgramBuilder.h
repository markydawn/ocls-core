/**
  *  This file is part of the OpenCL-ConsLaws framework
  *  Copyright (C) 2014, 2015 Jens Kristoffer Reitan Markussen
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */


#ifndef PROGRAM_BUILDER_H
#define PROGRAM_BUILDER_H

#include <functional>
#include <sys/wait.h>

#include "BoundaryFunction.h"
#include "Program.h"
#include "CLSSource.h"
#include "Domain.h"
#include "KernelResources.h"

namespace ocls {

// Forward dec.
class Data;

/**
 *
 *
 */
class ProgramBuilder{
public:
    /**
     *
     */
    ProgramBuilder(ComputeContext* compute_context);
    /**
     *
     */
    ~ProgramBuilder();
    
    /**
     *
     */
    Program* createInitialCondProgram(const Domain* domain, const CLSSource::Function* function_);

    /**
    * Create memcpy from buffer to texture program
    */
    Program* createMemcpyBufferToTextureProgram(size_t texture_dim);

    /**
    *
    */
    Program* createFindMaximumProgram(DeviceType type);

    /**
    *
    */
    Program* createFindMinimumProgram(DeviceType type);

    /**
    *
    */
    Program* createUnpaddedToPaddedProgram(int dim);

    /**
    *
    */
    Program* createPaddedToUnpaddedProgram(int dim);

    /**
    *
    */
    Program* createBoundaryProgram(const Domain* domain, const CLSSource::Function* function, OnBoundary boundary);

    /**
    * Create flux evaluator program
    */
    Program* createFluxProgram(const Domain* domain, const CLSSource::Function* function, size_t* return_values);

    /**
    * Create time integrator program
    */
    Program* createTimeIntegratorProgram(const Domain* domain, const CLSSource::Function* function, size_t* return_values);

    /**
     * Create source function program
     */
    Program* createSourceProgram(const Domain* domain, const CLSSource::Function* function, size_t* return_values);
    
private:

    Program* createBoundaryProgram1D(const CLSSource::Function* function, const Domain* domain, OnBoundary boundary);
    Program* createBoundaryProgram2D(const CLSSource::Function* function, const Domain* domain, OnBoundary boundary);
    Program* createBoundaryProgram3D(const CLSSource::Function* function, const Domain* domain, OnBoundary boundary);
    std::string createKernel(const CLSSource::Function *function, size_t *return_count = nullptr, bool early_exit = false);
    char toHex(int i);

    std::string appendDependencies(const Domain* domain, const CLSSource::Function* function);
    std::string getBoundaryConstantsDefinitions(OnBoundary boundary);
    std::string getDomainDefinitions(const Domain* domain);
    std::string appendGlobalsToBlock(const Domain* domain, std::string block);
    std::string buildSignature(const CLSSource::Function* function);

    bool isPointerType(CLSSource::Variable* variable);
private:
    ComputeContext* m_compute_context;
};

} //cls

#endif //PROGRAM_BUILDER_H
