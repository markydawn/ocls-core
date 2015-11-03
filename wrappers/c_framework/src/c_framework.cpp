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



#include <c_structures.h>
#include "c_framework.h"
#include "Framework.h"

extern "C" {

struct CallableFunctionHandle{
    ocls::CallableFunction* function;
};

const char* oclsGetError(void){
    return &error_buffer[0];
}

int oclsInit(Framework *framework) {
    try {
        *framework = reinterpret_cast<PointerHandle> (new ocls::Framework());
    }catch(std::exception& e){
        *framework = NULL;
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsInitByType(Framework* framework, int type) {
    try {
        *framework = reinterpret_cast<PointerHandle> (new ocls::Framework((ocls::DeviceType) type));
    }catch(std::exception& e){
        *framework = NULL;
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsClean(Framework framework) {
    try {
        delete reinterpret_cast<ocls::Framework *>(framework);
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsLoadFromFile(Framework framework, const char *path, CLSSource *source_out) {
    try {
        *source_out = reinterpret_cast<PointerHandle>(reinterpret_cast<ocls::Framework *>(framework)->getSourceLoader()->loadFromFile(std::string(path)));
    } catch(std::exception& e){
        strcpy(error_buffer, e.what());
        *source_out = NULL;
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsLoadFromString(Framework framework, const char *src, CLSSource *source_out) {
    try {
        *source_out = reinterpret_cast<PointerHandle>(reinterpret_cast<ocls::Framework *>(framework)->getSourceLoader()->loadFromString(std::string(src)));
    } catch(std::exception& e){
        strcpy(error_buffer, e.what());
        *source_out = NULL;
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsCreateData(Framework framework, Domain domain, const char *ref, Data *data_out) {
    try {
        *data_out = reinterpret_cast<PointerHandle>(reinterpret_cast<ocls::Framework *>(framework)
                ->createData(*reinterpret_cast<ocls::Domain *>(&domain), std::string(ref)));
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        *data_out = NULL;
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsCreateDataFromFunction(Framework framework, Domain domain, const char *ref, CLSSource src, const char *function, Data *data_out) {
    try {
        *data_out = reinterpret_cast<PointerHandle>(reinterpret_cast<ocls::Framework *>(framework)
                ->createData(*reinterpret_cast<ocls::Domain *>(&domain),
                std::string(ref), reinterpret_cast<ocls::CLSSource *>(src)->getFunction(std::string(function))));
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        *data_out = NULL;
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsCreateBoundaryFunction(Framework framework, Domain domain, CLSSource src, const char *function, BoundaryFunction *func_out) {
    try {
        (*func_out) = (CallableFunction) malloc(sizeof(CallableFunctionHandle)); //< TODO: Fix leak

        ocls::BoundaryFunction func = reinterpret_cast<ocls::Framework *>(framework)->createBoundaryFunction(*reinterpret_cast<ocls::Domain *>(&domain),
                reinterpret_cast<ocls::CLSSource *>(src)->getFunction(std::string(function)));

        (*func_out)->function = new ocls::BoundaryFunction(func); //< TODO: Fix leak
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsCreateFluxFunction(Framework framework, Domain domain, CLSSource src, const char *function, FluxFunction *func_out) {
    try {
        (*func_out) = (CallableFunction) malloc(sizeof(CallableFunctionHandle)); //< TODO: Fix leak

        ocls::FluxFunction func = reinterpret_cast<ocls::Framework *>(framework)->createFluxFunction(*reinterpret_cast<ocls::Domain *>(&domain),
                reinterpret_cast<ocls::CLSSource *>(src)->getFunction(std::string(function)));

        (*func_out)->function = new ocls::FluxFunction(func); //< TODO: Fix leak
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

int oclsCreateIntegratorFunction(Framework framework, Domain domain, CLSSource src, const char *function, IntegratorFunction *func_out) {
    try {
        (*func_out) = (CallableFunction) malloc(sizeof(CallableFunctionHandle)); //< TODO: Fix leak

        ocls::IntegratorFunction func = reinterpret_cast<ocls::Framework *>(framework)->createIntegratorFunction(*reinterpret_cast<ocls::Domain *>(&domain),
                reinterpret_cast<ocls::CLSSource *>(src)->getFunction(std::string(function)));

        (*func_out)->function = new ocls::IntegratorFunction(func); //< TODO: Fix leak
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

  int oclsCreateSourceFunction(Framework framework, Domain domain, CLSSource src, const char* function, IntegratorFunction* func_out) {
    // looks like there is lots of leaks above. Try to fix them too.
    // is it maybe just to fix the deconstructors?

    // just the same as above for integratorfunction. only difference is the contents of SourceFunction
    try {
      (*func_out) = (CallableFunction)malloc(sizeof(CallableFunctionHandle)); //< has leaks. TODO: fix
      ocls::IntegratorFunction func = reinterpret_cast<ocls::Framework *>(framework)->createSourceFunction(*reinterpret_cast<ocls::Domain*>(&domain), reinterpret_cast<ocls::CLSSource *>(src)->getFunction(std::string(function)));
      (*func_out)->function = new ocls::SourceFunction(func); //< omg! has leaks! TODO: fix
    }catch(std::exception& e) {
      strcpy(error_buffer, e.what());
      return ERROR_FATAL;
    }
    return ERROR_OK;
  }

int oclsCall(CallableFunction function, size_t length, Parameter* args, Data* return_out, int* return_length) {
    try {
        ocls::ReturnType returnType;
        returnType = function->function->call(length, reinterpret_cast<ocls::Program::Parameter *>(args));

        if(return_out != NULL) {
            ocls::Collection collection = returnType.asCollection();
            if (return_length != NULL){
                *return_length = (int)collection.size();
            }
            for (int i = 0; i < collection.size(); ++i) {
                return_out[i] = reinterpret_cast<PointerHandle>(collection[i]);
            }
        }
    }catch(std::exception& e){
        strcpy(error_buffer, e.what());
        return ERROR_FATAL;
    }
    return ERROR_OK;
}

}
