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


%module ocls_framework_swig
%include cpointer.i
%include typemaps.i
%include cdata.i
%include exception.i
%include carrays.i
%include cstring.i

%{
#define SWIG_FILE_WITH_INIT
#include "c_framework.h"
#include "c_domain.h"
#include "c_structures.h"
#include "c_analyser.h"
%}

%include numpy.i
%init %{
import_array();
%}

typedef struct {
    float min;
    float max;
}oclsBoundary;
typedef oclsBoundary Boundary;

typedef struct {
    unsigned short dimensions;
    Boundary bounds[3];
    size_t mesh[3];
    unsigned short ghost_cells;
}oclsDomain;
typedef oclsDomain Domain;

struct oclsPtrHandle{};
typedef struct oclsPtrHandle* PointerHandle;

typedef PointerHandle Data;
typedef PointerHandle CLSSource;
typedef PointerHandle Framework;

struct CallableFunctionHandle;
typedef struct CallableFunctionHandle* CallableFunction;

typedef CallableFunction BoundaryFunction;
typedef CallableFunction FluxFunction;
typedef CallableFunction IntegratorFunction;

// source function
typedef CallableFunction SourceFunction;

typedef struct {
    int count;

    float total_time;
    float total_api_overhead;
    float total_execution_time;

    float average_time;
    float average_api_overhead;
    float average_execution_time;
} KernelAnalysis;

typedef struct {
    int count;

    double total_time ;
    long long total_bytes;
    double total_rate;

    double average_time;
    long long average_bytes;
    double average_rate;
} MemEntry;

typedef struct  {
    MemEntry device_device;
    MemEntry device_host;
    MemEntry host_device;
} MemoryAnalysis;

%array_class(float, floatArray);
%array_class(size_t, size_tArray);
%array_class(Parameter, parameterArray);

%extend oclsParameter {
    oclsParameter(PyObject* value, size_t size) {
        Parameter *p;
        p = (Parameter *) malloc(sizeof(Parameter));
        p->allocation = 0;
        p->value = convert_to_void(value, &p->allocation);
        p->size = size;

        return p;
    }

	oclsParameter() {
		Parameter *p;
		p = (Parameter *) malloc(sizeof(Parameter));
		p->allocation = 0;
		return p;
	}
	~oclsParameter() {
	    if($self->allocation == 1){
	        free($self->value);
        }
		free($self);
	}
};

%{
/* Define a helper function */
static void *
convert_to_void(PyObject *input, int* allocation) {
  void *result = NULL;
  if (PyInt_Check(input)) {
      int tmp = (int)PyInt_AsLong(input);
      result = malloc(sizeof(int));
      *((int*)result) = tmp;
      *allocation = 1;
  }else if (PyFloat_Check(input)) {
      float tmp = (float)PyFloat_AsDouble(input);
      result = malloc(sizeof(float));
      *((float*)result) = tmp;
      *allocation = 1;
  }else {
      PyErr_SetString(PyExc_RuntimeError,"Everything is not OK, Unknown type in typemap");
  }
  return result;
}
%}

%typemap(in) void* value  %{
    int allocation;
    $1 = convert_to_void($input, &allocation);
    if (!$1) return NULL;
%}


%typemap(memberin) void* value %{
  $1 = $input;
  $self->allocation = allocation;
%}


typedef struct {
    void* value;
    size_t size;
    int allocation;
}oclsParameter;
typedef oclsParameter Parameter;

%rename(oclsInit) Init;
%inline %{
Framework Init(void) {
    Framework temp;
    int err = oclsInit(&temp);
    if(err != ERROR_OK){
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return temp;
}
%}

%rename(oclsInitByType) InitByType;
%inline %{
Framework InitByType(int type) {
    Framework temp;
    int err = oclsInitByType(&temp, type);
    if(err != ERROR_OK){
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return temp;
}
%}

%rename(oclsClean) Clean;
%inline %{
void Clean(Framework framework) {
    int err = oclsClean(framework);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsLoadFromFile) LoadFromFile;
%inline %{
CLSSource LoadFromFile(Framework framework, const char* path) {
    CLSSource src;
    int err = oclsLoadFromFile(framework, path, &src);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return src;
}
%}

%rename(oclsLoadFromString) LoadFromString;
%inline %{
CLSSource LoadFromString(Framework framework, const char* src) {
    CLSSource srcr;
    int err = oclsLoadFromString(framework, src, &srcr);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return srcr;
}
%}

%rename(oclsCreateData) CreateData;
%inline %{
Data CreateData(Framework framework, Domain domain, const char* ref) {
    Data data;
    int err = oclsCreateData(framework, domain, ref, &data);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return data;
}
%}

%rename(oclsCreateDataFromFunction) CreateDataFromFunction;
%inline %{
Data CreateDataFromFunction(Framework framework, Domain domain, const char* ref, CLSSource src, const char* function) {
    Data data;
    int err = oclsCreateDataFromFunction(framework, domain, ref, src, function, &data);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return data;
}
%}

%rename(oclsCreateBoundaryFunction) CreateBoundaryFunction;
%inline %{
BoundaryFunction CreateBoundaryFunction(Framework framework, Domain domain, CLSSource src, const char* function) {
    BoundaryFunction func;
    int err = oclsCreateBoundaryFunction(framework, domain, src, function, &func);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return func;
}
%}

%rename(oclsCreateFluxFunction) CreateFluxFunction;
%inline %{
FluxFunction CreateFluxFunction(Framework framework, Domain domain, CLSSource src, const char* function) {
    FluxFunction func;
    int err = oclsCreateFluxFunction(framework, domain, src, function, &func);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return func;
}
%}

%rename(oclsCreateIntegratorFunction) CreateIntegratorFunction;
%inline %{
IntegratorFunction CreateIntegratorFunction(Framework framework, Domain domain, CLSSource src, const char* function) {
    IntegratorFunction func;
    int err = oclsCreateIntegratorFunction(framework, domain, src, function, &func);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return func;
}
%}

// source function
%rename(oclsCreateSourceFunction) CreateSourceFunction;
%inline %{
  SourceFunction CreateSourceFunction(Framework framework, Domain domain, CLSSource src, const char* function) {
    SourceFunction func;
    int err = oclsCreateSourceFunction(framework, domain, src, function, &func);

    if(err != ERROR_OK) 
      PyErr_SetString(PyExc_RuntimeError, oclsGetError());

    return func;
  }
%}

%typemap(in, numinputs=0) int* count (int temp) {
    $1 = &temp;
}
%typemap(out) Data* Call {
  int i;
  //$1, $1_dim0, $1_dim1
  $result = PyList_New(*arg4);
  for (i = 0; i < *arg4; i++) {
    PyObject *o = SWIG_NewPointerObj(SWIG_as_voidptr($1[i]), SWIGTYPE_p_oclsPtrHandle, 0 |  0 );
    PyList_SetItem($result,i,o);
  }
  PyMem_Free($1);
}

%rename(oclsCall) Call;
%inline %{
Data* Call(CallableFunction function, size_t length, Parameter* args, int* count) {
    Data* d = (Data*)PyMem_Malloc(sizeof(Data)*25); //TODO: unsafe constant, might be more then 25 return values.
    int err = oclsCall(function, length, args, &d[0], count);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return d;
}
%}


int oclsSetDimensions(Domain* domain, const unsigned short dimensions_);
int oclsSetBoundsX(Domain* domain, const float left_, const float right_);
int oclsSetBoundsXY(Domain* domain, const float left_, const float right_, const float bottom_, const float top_);
int oclsSetBoundsXYZ(Domain* domain, const float left_, const float right_, const float bottom_, const float top_, const float front_, const float back_);
int oclsSetMeshX(Domain* domain, const size_t x_);
int oclsSetMeshXY(Domain* domain, const size_t x_, const size_t y_);
int oclsSetMeshXYZ(Domain* domain, const size_t x_, const size_t y_, const size_t z_);
int oclsSetGhostCells(Domain* domain, const unsigned short ghost_cells_);
float oclsGetDelta(Domain domain, const unsigned short dimension);



%apply (int* DIM1, float** ARGOUTVIEW_ARRAY1) {(int* length, float** ptr_out)};
%apply (int* DIM1, int* DIM2, float** ARGOUTVIEW_ARRAY2) {(int* length_x, int* length_y, float** ptr_out)};
%apply (int* DIM1, int* DIM2, int* DIM3, float** ARGOUTVIEW_ARRAY3) {(int* length_x, int* length_y, int* length_z, float** ptr_out)};

%rename(oclsGetData1D) GetData1D;
%inline %{
void GetData1D(Data d, int* length, float** ptr_out) {
    size_t bounds[3];
    int err = oclsGetDataPointerWithBounds(d, bounds, ptr_out);

    *length = (int)bounds[0];
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

%rename(oclsGetData2D) GetData2D;
%inline %{
void GetData2D(Data d, int* length_x, int* length_y, float** ptr_out) {
    size_t bounds[3];
    int err = oclsGetDataPointerWithBounds(d, bounds, ptr_out);

    *length_y = (int)bounds[0];
    *length_x = (int)bounds[1];
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

%rename(oclsGetData3D) GetData3D;
%inline %{
void GetData3D(Data d, int* length_x, int* length_y, int* length_z, float** ptr_out) {
    size_t bounds[3];
    int err = oclsGetDataPointerWithBounds(d, bounds, ptr_out);

    *length_z = (int)bounds[0];
    *length_y = (int)bounds[1];
    *length_x = (int)bounds[2];
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

Parameter oclsGetParameter(Data d);


%apply (int DIM1, float* IN_ARRAY1) {(int length, float* ptr_in)};
%apply (int DIM1, int DIM2, float* IN_ARRAY2) {(int length_x, int length_y, float* ptr_in)};
%apply (int DIM1, int DIM2, int DIM3, float* IN_ARRAY3) {(int length_x, int length_y, int length_z, float* ptr_in)};

%rename(oclsSetData1D) SetData1D;
%inline %{
void SetData1D(Data d, int length, float* ptr_in) {
    size_t bounds[3] = {(size_t)length, 1, 1};
    int err = oclsUpload(d, ptr_in, &bounds[0], NULL);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

%rename(oclsSetData2D) SetData2D;
%inline %{
void SetData2D(Data d, int length_x, int length_y, float* ptr_in) {
    size_t bounds[3] = {(size_t)length_y, (size_t)length_x, 1};
    int err = oclsUpload(d, ptr_in, &bounds[0], NULL);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

%rename(oclsSetData3D) SetData3D;
%inline %{
void SetData3D(Data d, int length_x, int length_y, int length_z, float* ptr_in) {
    size_t bounds[3] = {(size_t)length_z, (size_t)length_y, (size_t)length_x};
    int err = oclsUpload(d, ptr_in, &bounds[0], NULL);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

}
%}

/*
float oclsAtX(Data d, size_t x);
float oclsAtXY(Data d, size_t x, size_t y);
float oclsAtXYZ(Data d, size_t x, size_t y, size_t z);
*/

%rename(oclsAtX) AtX;
%inline %{
float AtX(Data d, size_t x) {
    float value;
    int err = oclsAtXSafe(d, x, &value);
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
    return value;
}
%}

%rename(oclsAtXY) AtXY;
%inline %{
float AtXY(Data d, size_t x, size_t y) {
    float value;
    int err = oclsAtXYSafe(d, x,y, &value);
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
    return value;
}
%}

%rename(oclsAtXYZ) AtXYZ;
%inline %{
float AtXYZ(Data d, size_t x, size_t y, size_t z) {
    float value;
    int err = oclsAtXYZSafe(d, x,y,z, &value);
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
    return value;
}
%}

int oclsDimensions(Data d);

%rename(oclsCopy) Copy;
%inline %{
void Copy(Data src, Data dest) {
    int err = oclsCopy(src, dest);
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}


%rename(oclsFindMax) FindMax;
%inline %{
float FindMax(Data d) {
    float value;
    int err = oclsFindMax(d, &value);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return value;
}
%}

%rename(oclsFindMin) FindMin;
%inline %{
float FindMin(Data d) {
    float value;
    int err = oclsFindMin(d, &value);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }

    return value;
}
%}

%rename(oclsSize) Size;
%inline %{
void Size(Data d, int* len, size_t** size) {
    int err = oclsSize(d, *size);
    *len = 3;
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsAnalyserStart) AnalyserStart;
%inline %{
void AnalyserStart(void) {
    int err = oclsAnalyserStart();
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsAnalyserEnd) AnalyserEnd;
%inline %{
void AnalyserEnd(void) {
    int err = oclsAnalyserEnd();
    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%cstring_output_allocate(char** output, free(*$1))
%rename(oclsAnalyserRead) AnalyserRead;
%inline %{
void AnalyserRead(char** output) {
    int err = oclsAnalyserRead(output);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsAnalyserMemoryRead) AnalyserMemoryRead;
%inline %{
void AnalyserMemoryRead(char** output) {
    int err = oclsAnalyserMemoryRead(output);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsAnalyserKernelRead) AnalyserKernelRead;
%inline %{
void AnalyserKernelRead(const char *kernel_name, char** output) {
    int err = oclsAnalyserKernelRead(kernel_name, output);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
}
%}

%rename(oclsAnalyserMemory) AnalyserMemory;
%inline %{
MemoryAnalysis* AnalyserMemory(void) {
    MemoryAnalysis* analysis;
    int err = oclsAnalyserMemory(&analysis);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
    return analysis;
}
%}

%rename(oclsAnalyserKernel) AnalyserKernel;
%inline %{
KernelAnalysis* AnalyserKernel(const char *kernel_name) {
    KernelAnalysis* analysis;
    int err = oclsAnalyserKernel(kernel_name, &analysis);

    if(err != ERROR_OK) {
        PyErr_SetString(PyExc_RuntimeError,oclsGetError());
    }
    return analysis;
}
%}
