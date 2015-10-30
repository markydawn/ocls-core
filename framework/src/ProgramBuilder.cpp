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


#include "ProgramBuilder.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <Data.h>
#include <Utility.h>
#include <BoundaryFunction.h>
#include <sys/wait.h>

namespace ocls {
ProgramBuilder::ProgramBuilder(ComputeContext* compute_context):
        m_compute_context(compute_context){

}

ProgramBuilder::~ProgramBuilder(){

}

Program* ProgramBuilder::createInitialCondProgram(const Domain* domain, const CLSSource::Function* function_){
    std::string source = appendDependencies(domain, function_);

    std::stringstream ss;
    std::string type = function_->signature.substr(0, function_->signature.find_first_of(" \t\n"));
    std::string func = std::string("store");

    ss << "__kernel void "<< function_->name <<"_kernel(__global " <<
            type << "* Q)" << "{ "


            << func << "(Q," <<
            function_->name << "(),(uint";
    if (domain->getDimensions() == 1) {
        ss << ")get_global_id(0));";
    }else if (domain->getDimensions() == 2) {
        ss << "2) (get_global_id(0), get_global_id(1)));";
    } else  {
        ss << "3) (get_global_id(0), get_global_id(1), get_global_id(2)));";
    }
    ss << "}";

    source.append(ss.str());

    std::string args = getDomainDefinitions(domain);
    //args.append("-Werror ");

    return new Program(m_compute_context, *domain, function_->name, source, args);;
}

std::string ProgramBuilder::getDomainDefinitions(const Domain* domain){
    std::stringstream ss;
    size_t multiple = 32;

    //std::cout << "Creating domain def" << std::endl;
    //std::cout << "Dim" << domain->getDimensions() << std::endl;
    //std::cout << "GC" << domain->getGhostCells() << std::endl;
    //std::cout << "Mesh_x" << domain->getMesh(Domain::Dim::X) << std::endl;
    //std::cout << "Mesh_y" << domain->getMesh(Domain::Dim::Y) << std::endl;
    //std::cout << "Mesh_z" << domain->getMesh(Domain::Dim::Z) << std::endl;
    //std::cout << "Meshgc_x" << domain->getMeshWithGC(Domain::Dim::X) << std::endl;
    //std::cout << "Meshgc_y" << domain->getMeshWithGC(Domain::Dim::Y) << std::endl;
    //std::cout << "Meshgc_z" << domain->getMeshWithGC(Domain::Dim::Z) << std::endl;
    //std::cout << std::endl;


    ss << "-D _DIMENSIONS_=" << domain->getDimensions() << " "
            << "-D Nx=" << domain->getMesh(Domain::Dim::X) << " "
            << "-D Ny=" << domain->getMesh(Domain::Dim::Y) << " "
            << "-D Nz=" << domain->getMesh(Domain::Dim::Z) << " "
            << "-D NxG=" << roundUp(domain->getMeshWithGC(Domain::Dim::X),multiple) << " "
            << "-D NyG=" << (domain->getDimensions() >= 2 ? roundUp(domain->getMeshWithGC(Domain::Dim::Y),multiple) : 1) << " "
            << "-D NzG=" << (domain->getDimensions() == 3 ? roundUp(domain->getMeshWithGC(Domain::Dim::Z),multiple) : 1) << " "
            << "-D GC=" << domain->getGhostCells() << " "
            << "-D DX=" << domain->getDelta(Domain::Dim::X) << " "
            << "-D DY=" << domain->getDelta(Domain::Dim::Y) << " "
            << "-D DZ=" << domain->getDelta(Domain::Dim::Z) << " ";
    //std::cout << ss.str() << std::endl;
    //std::cout << std::endl;
    return ss.str();
}

std::string ProgramBuilder::appendGlobalsToBlock(const Domain* domain, std::string block){
    std::stringstream ss;
    ss.setf(std::ios_base::fixed,std::ios_base::floatfield);
    ss.precision(2);
    float dx = domain->getDelta(Domain::Dim::X);
    float dy = domain->getDelta(Domain::Dim::Y);
    float dz = domain->getDelta(Domain::Dim::Z);
    dx*=(domain->getGhostCells());
    dy*=(domain->getGhostCells());
    dz*=(domain->getGhostCells());

    bool cell_found = block.find("cell") != std::string::npos;
    bool pos_found = block.find("pos") != std::string::npos;

    if( pos_found || cell_found )
        ss << std::endl << "\tuint3 cell = (uint3)(get_global_id(0),get_global_id(1),get_global_id(2));" << std::endl;

    if(pos_found) {
        ss << "\tfloat3 pos = (float3)(mix(" << domain->getBounds(Domain::Dim::X).min - dx << "f," << domain->getBounds(Domain::Dim::X).max + dx
                << "f,(float)cell.x/" << ((domain->getMeshWithGC(Domain::Dim::X) - 1 == 0) ? 1 : domain->getMeshWithGC(Domain::Dim::X) - 1) << ".0f),"
                << "mix(" << domain->getBounds(Domain::Dim::Y).max + dy << "f," << domain->getBounds(Domain::Dim::Y).min - dy
                << "f,(float)cell.y/" << ((domain->getMeshWithGC(Domain::Dim::Y) - 1 == 0) ? 1 : domain->getMeshWithGC(Domain::Dim::Y) - 1) << ".0f),"
                << "mix(" << domain->getBounds(Domain::Dim::Z).min - dz << "f," << domain->getBounds(Domain::Dim::Z).max + dz
                << "f,(float)cell.z/" << ((domain->getMeshWithGC(Domain::Dim::Z) - 1 == 0) ? 1 : domain->getMeshWithGC(Domain::Dim::Z) - 1) << ".0f));" << std::endl;
    }
    return block.insert(1, ss.str());
}

Program *ProgramBuilder::createMemcpyBufferToTextureProgram(size_t texture_dim) {
    if(texture_dim == 3 && !m_compute_context->isExtensionAvailable("cl_khr_3d_image_writes")) {
        logger->log(Logger::WARNING, "Unable to create 3D image write program, it is unsupported by compute device");
        return NULL;
    }

    std::string source = "";
    source.append(header::source);
    source.append(helper::source);

    source.append(utility_kernels::source);

    Domain domain(texture_dim);

    std::string args = getDomainDefinitions(&domain);
    //args.append("-Werror ");

    Program* prog = new Program(m_compute_context, domain, "memcpy_buf_tex", source, args);

    return prog;
}

Program *ProgramBuilder::createFindMaximumProgram(DeviceType type) {
    std::string source = "";

    source.append(default_programs::source);

    std::stringstream ss;
    ss << "-D FIND_MAX=1 ";
    ss << "-D __DEVICE_TYPE__=" << (int)type;
    std::string args = ss.str();
    //args.append("-Werror ");

    return new Program(m_compute_context, Domain(1), "reduce", source, args);
}

Program *ProgramBuilder::createFindMinimumProgram(DeviceType type) {
    std::string source = "";

    source.append(default_programs::source);

    std::stringstream ss;
    ss << "-D FIND_MIN=1 ";
    ss << "-D __DEVICE_TYPE__=" << (int)type;
    std::string args = ss.str();
    //args.append("-Werror ");

    return new Program(m_compute_context, Domain(1), "reduce", source, args);
}

Program *ProgramBuilder::createUnpaddedToPaddedProgram(int dim) {
    std::string source = default_programs::source;

    if(dim == 1)
        return new Program(m_compute_context, Domain(1), "unpadded_to_padded_1D", source, "");
    if(dim == 2)
        return new Program(m_compute_context, Domain(1), "unpadded_to_padded_2D", source, "");
    if(dim == 3)
        return new Program(m_compute_context, Domain(1), "unpadded_to_padded_3D", source, "");

    return NULL;
}

Program *ProgramBuilder::createPaddedToUnpaddedProgram(int dim) {
    std::string source = default_programs::source;

    if(dim == 1)
        return new Program(m_compute_context, Domain(1), "padded_to_unpadded_1D", source, "");
    if(dim == 2)
        return new Program(m_compute_context, Domain(1), "padded_to_unpadded_2D", source, "");
    if(dim == 3)
        return new Program(m_compute_context, Domain(1), "padded_to_unpadded_3D", source, "");

    return NULL;
}

std::string ProgramBuilder::appendDependencies(const Domain* domain, const CLSSource::Function *function) {
    std::string headers = header::source;
    std::string helpers = helper::source;
    std::string limiters = limiters::source;
    std::string reconstructors = reconstructors::source;

    std::string source = "";
    source.append(headers);

    for (size_t i = 0; i < function->variable_dependencies.size(); i++) {
        CLSSource::Variable *var = function->variable_dependencies[i];
        std::stringstream ss;
        ss << "__constant " << var->type <<
                " " << var->name << " = " << var->value << ";" << std::endl;
        source.append(ss.str());
        source.append("\n");
    }


    std::vector<std::string> declared;

    for (size_t i = 0; i < function->structure_dependencies.size(); i++) {
        CLSSource::Structure *stct = function->structure_dependencies[i];
        bool already_declared = false;
        for (int k = 0; k < declared.size(); ++k) {
            if (declared[k] == stct->name){
                already_declared = true;
                break;
            }
        }

        if(already_declared){
            break;
        }
        std::stringstream ss;
        ss << "typedef struct {" << std::endl;
        std::map<std::string, CLSSource::Variable*>::iterator it;
        for (it = stct->variables.begin(); it != stct->variables.end(); ++it) {
            if(isPointerType(it->second)){
                ss << "__global ";
            }
            ss << it->second->type << " " << it->second->name << ";";
        }
        ss << "} " << stct->name << ";";

        declared.push_back(stct->name);
        source.append(ss.str());
        source.append("\n");
    }

    source.append(buildSignature(function));
    source.append(";\n");


    for (size_t i = 0; i < function->function_dependencies.size(); i++) {
        CLSSource::Function *func = function->function_dependencies[i];

        for (int j = 0; j < func->function_dependencies.size(); ++j) {
            CLSSource::Function *func2 = func->function_dependencies[j];

            source.append(buildSignature(func2));
            source.append(";\n");
        }

        source.append(buildSignature(func));
        source.append(";\n");
    }

    source.append(helpers);
    source.append(limiters);
    source.append(reconstructors);

    source.append(buildSignature(function));
    source.append(appendGlobalsToBlock(domain, function->block));
    source.append("\n");

    for (size_t i = 0; i < function->function_dependencies.size(); i++) {
        CLSSource::Function *func = function->function_dependencies[i];

        for (int j = 0; j < func->function_dependencies.size(); ++j) {
            CLSSource::Function *func2 = func->function_dependencies[j];

            source.append(buildSignature(func2));
            source.append(appendGlobalsToBlock(domain, func2->block));
            source.append(";\n");
        }

        source.append(buildSignature(func));
        source.append(appendGlobalsToBlock(domain, func->block));
        source.append("\n");
    }

    return source;
}

bool ProgramBuilder::isPointerType(CLSSource::Variable *variable) {
    return variable->type.find('*') != std::string::npos;
}


Program *ProgramBuilder::createBoundaryProgram(const Domain *domain, const CLSSource::Function *function, OnBoundary boundary) {
    if(domain->getDimensions() == 1)
        return createBoundaryProgram1D(function, domain, boundary);
    if(domain->getDimensions() == 2)
        return createBoundaryProgram2D(function, domain, boundary);
    if(domain->getDimensions() == 3)
        return createBoundaryProgram3D(function, domain, boundary);
    return NULL;
}

Program *ProgramBuilder::createBoundaryProgram1D(const CLSSource::Function *function, const Domain* domain, OnBoundary boundary) {
    Domain tmp(1);
    tmp.setGhostCells(0);
    tmp.setMesh(domain->getGhostCells());

    std::string source = getBoundaryConstantsDefinitions(boundary);
    source.append(appendDependencies(&tmp, function));

    source.append(createKernel(function));

    std::string args = getDomainDefinitions(domain);
    args.append("-D _BOUNDARY_PROGRAM_");
    //args.append("-Werror ");

    return new Program(m_compute_context, tmp, function->name, source, args);
}

Program *ProgramBuilder::createBoundaryProgram2D(const CLSSource::Function *function, const Domain* domain, OnBoundary boundary) {
    Domain tmp(2);
    tmp.setGhostCells(0);
    if(boundary == POS_X || boundary == NEG_X)
        tmp.setMesh(domain->getGhostCells(),domain->getMeshWithGC(Domain::Dim::Y));
    else
        tmp.setMesh(domain->getMeshWithGC(Domain::Dim::X),domain->getGhostCells());

    std::string source = getBoundaryConstantsDefinitions(boundary);
    source.append(appendDependencies(&tmp, function));

    source.append(createKernel(function));

    std::string args = getDomainDefinitions(domain);
    args.append("-D _BOUNDARY_PROGRAM_");
    //args.append("-Werror ");

    return new Program(m_compute_context, tmp, function->name, source, args);
}

Program *ProgramBuilder::createBoundaryProgram3D(const CLSSource::Function *function, const Domain* domain, OnBoundary boundary) {
    Domain tmp(3);
    tmp.setGhostCells(0);
    if(boundary == POS_X || boundary == NEG_X)
        tmp.setMesh(domain->getGhostCells(),domain->getMeshWithGC(Domain::Dim::Y),domain->getMeshWithGC(Domain::Dim::Z));
    else if(boundary == POS_Y || boundary == NEG_Y)
        tmp.setMesh(domain->getMeshWithGC(Domain::Dim::X),domain->getGhostCells(),domain->getMeshWithGC(Domain::Dim::Z));
    else
        tmp.setMesh(domain->getMeshWithGC(Domain::Dim::X),domain->getMeshWithGC(Domain::Dim::Y),domain->getGhostCells());

    std::string source = getBoundaryConstantsDefinitions(boundary);
    source.append(appendDependencies(&tmp, function));

    source.append(createKernel(function));

    std::string args = getDomainDefinitions(domain);
    args.append("-D _BOUNDARY_PROGRAM_");
    //args.append("-Werror ");

    return new Program(m_compute_context, tmp, function->name, source, args);
}

std::string ProgramBuilder::getBoundaryConstantsDefinitions(OnBoundary boundary) {
    switch (boundary){
        case POS_X:
            return "__constant bool POS_X = 1;"
                    "__constant bool RIGHT = 1;"
                    "__constant bool EAST = 1;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
        case NEG_X:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 0;"
                    "__constant bool NEG_X = 1;"
                    "__constant bool LEFT = 1;"
                    "__constant bool WEST = 1;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
        case POS_Y:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 0;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 1;"
                    "__constant bool NORTH = 1;"
                    "__constant bool TOP = 1;"
                    "__constant bool UP = 1;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
        case NEG_Y:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 0;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 1;"
                    "__constant bool SOUTH = 1;"
                    "__constant bool BOTTOM = 1;"
                    "__constant bool DOWN = 1;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
        case POS_Z:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 0;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 1;"
                    "__constant bool FRONT = 1;"
                    "__constant bool NEAR = 1;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
        case NEG_Z:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 1;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 1;"
                    "__constant bool BACK = 1;"
                    "__constant bool FAR = 1;";
        default:
            return "__constant bool POS_X = 0;"
                    "__constant bool RIGHT = 0;"
                    "__constant bool EAST = 0;"
                    "__constant bool NEG_X = 0;"
                    "__constant bool LEFT = 0;"
                    "__constant bool WEST = 0;"
                    "__constant bool POS_Y = 0;"
                    "__constant bool NORTH = 0;"
                    "__constant bool TOP = 0;"
                    "__constant bool UP = 0;"
                    "__constant bool NEG_Y = 0;"
                    "__constant bool SOUTH = 0;"
                    "__constant bool BOTTOM = 0;"
                    "__constant bool DOWN = 0;"
                    "__constant bool POS_Z = 0;"
                    "__constant bool FRONT = 0;"
                    "__constant bool NEAR = 0;"
                    "__constant bool NEG_Z = 0;"
                    "__constant bool BACK = 0;"
                    "__constant bool FAR = 0;";
    }
}
std::string ProgramBuilder::buildSignature(const CLSSource::Function *function) {
    std::string source;

    source.append(function->returntype);
    source.append(" ");
    source.append(function->name);
    source.append("(");
    if(function->parameters.size() > 0) {
        for (int j = 0; j < function->parameters.size(); ++j) {
            if (isPointerType(function->parameters[j])) {
                source.append("__global ");
            }
            source.append(function->parameters[j]->type);
            source.append(" ");
            source.append(function->parameters[j]->name);
            if (function->parameters[j]->array_type) {
                std::stringstream stream;
                stream << "[" << function->parameters[j]->array_size << "]";
                source.append(stream.str());
            }

            if (j != function->parameters.size() - 1) {
                source.append(",");
            }
        }
    }else{
        source.append("void");
    }
    source.append(")");

    return source;
}
std::string ProgramBuilder::createKernel(const CLSSource::Function *function, size_t *return_count, bool early_exit) {
    std::string array_assignment = "";
    std::string return_assignment = "";
    size_t return_values = 0;

    std::stringstream ss;
    ss  << "__kernel void "<< function->name <<"_kernel(";

    for (int i = 0; i < function->parameters.size(); ++i) {
        if(function->parameters[i]->array_type){
            std::stringstream array_ss;
            array_ss << "__global " << function->parameters[i]->type << " " << function->parameters[i]->name << "[" << function->parameters[i]->array_size << "];";
            for (int j = 0; j < function->parameters[i]->array_size; ++j) {
                ss << "__global " << function->parameters[i]->type << " " << function->parameters[i]->name << "_" << j;
                array_ss << function->parameters[i]->name << "[" << j << "]=" << function->parameters[i]->name << "_" << j << ";";
                if (j != function->parameters[i]->array_size-1){
                    ss << ", ";
                }
            }
            array_assignment.append(array_ss.str());
        }else {
            if (isPointerType(function->parameters[i])) {
                ss << "__global ";
            }
            ss << function->parameters[i]->type << " " << function->parameters[i]->name;
        }
        if (i != function->parameters.size()-1) {
            ss << ", ";
        }
    }
    if(function->returntype != "void"){
        if(function->parameters.size() > 0){
            ss << ", ";
        }

        if(function->returntype == "float"){
            return_values = 1;
        }else{
            // float2, float3, ...
            if (function->returntype.size() == 6) {
                return_values = (size_t) std::atoi(
                        function->returntype.substr(function->returntype.size() - 1, 1).c_str());
            }else{
                return_values = (size_t) std::atoi(
                        function->returntype.substr(function->returntype.size() - 2, 2).c_str());
            }
        }

        std::stringstream return_ss;
        if(return_values == 1){
            ss << "__global float* r_0";
            return_ss << "store(r_0, value, " << std::endl;
            return_ss << "#if (_DIMENSIONS_ == 1)" << std::endl
                    << "(uint)(get_global_id(0)));" << std::endl;
            return_ss << "#elif (_DIMENSIONS_ == 2)" << std::endl
                    << "(uint2)(get_global_id(0),get_global_id(1)));" << std::endl;
            return_ss << "#elif (_DIMENSIONS_ == 3)" << std::endl
                    << "(uint3)(get_global_id(0),get_global_id(1),get_global_id(2)));" << std::endl;
            return_ss << "#endif" << std::endl;

        }else {
            for (int i = 0; i < return_values; ++i) {
                ss << "__global float* r_" << i;

                return_ss << "store(r_" << i << ", value.s" << toHex(i) << ", " << std::endl;
                return_ss << "#if (_DIMENSIONS_ == 1)" << std::endl
                        << "(uint)(get_global_id(0)));" << std::endl;
                return_ss << "#elif (_DIMENSIONS_ == 2)" << std::endl
                        << "(uint2)(get_global_id(0),get_global_id(1)));" << std::endl;
                return_ss << "#elif (_DIMENSIONS_ == 3)" << std::endl
                        << "(uint3)(get_global_id(0),get_global_id(1),get_global_id(2)));" << std::endl;
                return_ss << "#endif" << std::endl;


                if (i != return_values - 1) {
                    ss << ", ";
                }
            }
        }
        return_assignment.append(return_ss.str());
    }
    ss << "){" << std::endl;
    if(early_exit){
        ss << "#if (_DIMENSIONS_ >= 1)" << std::endl
                << "#if defined(_FLUX_PROGRAM_)" << std::endl
                << "if(get_global_id(0) < GC-1 || get_global_id(0) > Nx+GC-1) return;" << std::endl
                << "#elif defined(_INTEGRATOR_PROGRAM_)" << std::endl
                << "if(get_global_id(0) < GC || get_global_id(0) > Nx+GC-1) return;" << std::endl
                << "#endif" << std::endl;
        ss << "#endif" << std::endl;

        ss << "#if (_DIMENSIONS_ >= 2)" << std::endl
                << "#if defined(_FLUX_PROGRAM_)" << std::endl
                << "if(get_global_id(1) < GC-1 || get_global_id(1) > Ny+GC-1) return;" << std::endl
                << "#elif defined(_INTEGRATOR_PROGRAM_)" << std::endl
                << "if(get_global_id(1) < GC || get_global_id(1) > Ny+GC-1) return;" << std::endl
                << "#endif" << std::endl;
        ss << "#endif" << std::endl;

        ss << "#if (_DIMENSIONS_ == 3)" << std::endl
                << "#if defined(_FLUX_PROGRAM_)" << std::endl
                << "if(get_global_id(2) < GC-1 || get_global_id(2) > Nz+GC-1) return;" << std::endl
                << "#elif defined(_INTEGRATOR_PROGRAM_)" << std::endl
                << "if(get_global_id(2) < GC || get_global_id(2) > Nz+GC-1) return;" << std::endl
                << "#endif" << std::endl;
        ss << "#endif" << std::endl;
    }

    ss << array_assignment;

    if(return_values > 0){
        ss << function->returntype << " value=";
    }

    ss << function->name << "(";
    for (int i = 0; i < function->parameters.size(); ++i) {
        ss << function->parameters[i]->name;
        if (i != function->parameters.size()-1){
            ss << ", ";
        }
    }
    ss  << ");";

    if(return_values > 0) {
        ss << return_assignment;
    }

    ss  << "}";

    if(return_count != nullptr){
        *return_count = return_values;
    }

    return ss.str();
}

Program *ProgramBuilder::createFluxProgram(const Domain *domain, const CLSSource::Function *function, size_t* return_values) {
    std::string source = appendDependencies(domain, function);

    source.append(createKernel(function, return_values, true));

    std::string args = getDomainDefinitions(domain);
    args.append("-D _FLUX_PROGRAM_");
    //args.append("-Werror ");

    return new Program(m_compute_context, *domain, function->name, source, args);
}

Program *ProgramBuilder::createTimeIntegratorProgram(const Domain *domain, const CLSSource::Function *function, size_t* return_values) {
    std::string source = appendDependencies(domain, function);

    source.append(createKernel(function, return_values, true));

    std::string args = getDomainDefinitions(domain);
    args.append("-D _INTEGRATOR_PROGRAM_");
    //args.append("-Werror ");

    return new Program(m_compute_context, *domain, function->name, source, args);
}


  // because I use emacs,  we get (superior) emacs indentation here. fix this later
  Program* ProhramBuilder::createSourceProgram(const Domain *domain, const CLSSource::Function* function, size_t* return_values) {
    //TODO: make both boundary and function in one. Automatically reflect. That is probably the best solution
    std::string source = appendDependencies(domain, function);

    // get some inspiration or reuse stuff from boundary maybe?
  }


char ProgramBuilder::toHex(int i) {
    if (i < 10){
        return (char)(48+i); // ASCII codes numerals
    }else{
        return (char)(87+i); // and characters
    }
}

} //cls
