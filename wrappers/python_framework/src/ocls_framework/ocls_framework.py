#
#  This file is part of the OpenCL-ConsLaws framework
#  Copyright (C) 2014, 2015 Jens Kristoffer Reitan Markussen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


"""OCLS Framework Python API.

See module documentation for more details
"""

import ocls_framework_swig as c_api
import numbers
import numpy
import copy
import inspect

class Type:
    class GPU:
        pass
    class CPU:
        pass
    class ACCELERATOR:
        pass

    @staticmethod
    def conv(dev_type):
        if dev_type is Type.GPU:
            return 0
        if dev_type is Type.CPU:
            return 1
        if dev_type is Type.ACCELERATOR:
            return 2

class Framework:
    """ Class representing a framework context
    """
    def __init__(self, device_type=Type.GPU):
        """ Constructor """
        global __framework__
        __framework__ = self

        self.__ctx = c_api.oclsInitByType(Type.conv(device_type))

    def __del__(self):
        """ Destructor """
        c_api.oclsClean(self.__ctx)

    def loadFromFile(self, path):
        """ Load code from a file path """
        return c_api.oclsLoadFromFile(self.__ctx, path)

    def loadFromString(self, source):
        """ Load code from a string literal """
        return c_api.oclsLoadFromString(self.__ctx, source)

    def createData(self, domain, name="", source=None, function=None):
        """ Create a data object

        :returns:  Data -- New data object.
        """
        return Data(self, domain, name, source, function)

    def createBoundaryFunction(self, domain, src, function):
        """ Create a function type """
        return BoundaryFunction(self, domain, src, function)

    def createFluxFunction(self, domain, src, function):
        """ Create a function type """
        return FluxFunction(self, domain, src, function)

    def createIntegratorFunction(self, domain, src, function):
        """ Create a function type """
        return IntegratorFunction(self, domain, src, function)

    # source function
    def createSourceFunction(self, domain, src, function):
        """ Creates a source term function """
        return SourceFunction(self, domain, src, function)

# Defaults Context
__framework__ = Framework()

class Data:
    """ Class representing a data object
    """

    def __init__(self, framework=None, domain=None, name="", source=None, function=None, handle=None):
        if handle is not None:
            self.__handle = handle
        else:
            global __framework__
            if framework is None:
                framework = __framework__
            if domain is None:
                raise NotImplementedError()

            if source is None and function is None:
                new_handle = c_api.oclsCreateData(framework._Framework__ctx, domain._Domain__d, name)
            else:
                new_handle = c_api.oclsCreateDataFromFunction(framework._Framework__ctx, domain._Domain__d, name, source, function)

            self.__handle = new_handle

        self.__domain = domain

    def __param__(self):
        return c_api.oclsGetParameter(self.__handle)

    def __getitem__(self, pos):
        if isinstance(pos, int):
            return c_api.oclsAtX(self.__handle, pos)

        if len(pos) == 2:
            return c_api.oclsAtXY(self.__handle, pos[0], pos[1])

        return c_api.oclsAtXYZ(self.__handle, pos[0], pos[1], pos[2])

    def copy(self, other):
        c_api.oclsCopy(other._Data__handle, self.__handle)

    def max(self):
        return c_api.oclsFindMax(self.__handle)

    def min(self):
        return c_api.oclsFindMin(self.__handle)

    def set(self, arg):
        dim = c_api.oclsDimensions(self.__handle)

        if hasattr(arg, '__call__'):
            arg_c = len(inspect.getargspec(arg).args)
            if dim is 1:
                values = numpy.zeros(self.__domain.x_mesh+(self.__domain.ghost_cells*2)).astype('float32')
                xv = self.__domain.toLinspace()
                for x in range(self.__domain.x_mesh):
                    i = x+self.__domain.ghost_cells
                    if arg_c is 0:
                        values[i] = arg()
                    elif arg_c is 1:
                        values[i] = arg([xv[x]])
                    else:
                        values[i] = arg([xv[x]],[i])
                c_api.oclsSetData1D(self.__handle,values)
            elif dim is 2:
                values = numpy.zeros((self.__domain.y_mesh+(self.__domain.ghost_cells*2),
                        self.__domain.x_mesh+(self.__domain.ghost_cells*2))).astype('float32')
                xv, yv = self.__domain.toLinspace()
                for x in range(self.__domain.x_mesh):
                    for y in range(self.__domain.y_mesh):
                        i = x+self.__domain.ghost_cells
                        j = y+self.__domain.ghost_cells
                        if arg_c is 0:
                            values[j][i] = arg()
                        elif arg_c is 1:
                            values[j][i] = arg([xv[x],yv[y]])
                        else:
                            values[j][i] = arg([xv[x],yv[y]],[i,j])
                c_api.oclsSetData2D(self.__handle,values)
            else:
                values = numpy.zeros((self.__domain.z_mesh+(self.__domain.ghost_cells*2),
                        self.__domain.y_mesh+(self.__domain.ghost_cells*2),
                        self.__domain.x_mesh+(self.__domain.ghost_cells*2))).astype('float32')
                xv, yv, zv = self.__domain.toLinspace()
                for x in range(self.__domain.x_mesh):
                    for y in range(self.__domain.y_mesh):
                        for z in range(self.__domain.z_mesh):
                            i = x+self.__domain.ghost_cells
                            j = y+self.__domain.ghost_cells
                            k = z+self.__domain.ghost_cells
                            if arg_c is 0:
                                values[k][j][i] = arg()
                            elif arg_c is 1:
                                values[k][j][i] = arg([xv[x],yv[y],zv[z]])
                            else:
                                values[k][j][i] = arg([xv[x],yv[y],zv[z]],[i,j,k])
                c_api.oclsSetData3D(self.__handle,values)

        else:
            if dim is 1:
                c_api.oclsSetData1D(self.__handle,arg)
            elif dim is 2:
                c_api.oclsSetData2D(self.__handle,arg)
            else:
                c_api.oclsSetData3D(self.__handle,arg)

    def toArray(self):
        dim = c_api.oclsDimensions(self.__handle)
        if dim is 1:
            return c_api.oclsGetData1D(self.__handle)
        elif dim is 2:
            return c_api.oclsGetData2D(self.__handle)
        else:
            return c_api.oclsGetData3D(self.__handle)

class CallableFunction:
    def __init__(self, handle, domain):
        self.__handle = handle
        self.__domain = domain

    def __call__(self, *args):
        c = len(args)
        params = c_api.parameterArray(c)
        p = []
        for i in range(0, c):
            if isinstance(args[i], Data):
                params[i] = args[i].__param__()
            else:
                if isinstance(args[i], numbers.Integral):
                    p.append(c_api.oclsParameter(args[i], numpy.iinfo(args[i]).dtype.itemsize/2))
                    params[i] = p[len(p)-1]
                else:
                    p.append(c_api.oclsParameter(args[i], numpy.finfo(args[i]).dtype.itemsize/2))
                    params[i] = p[len(p)-1]

        handles = c_api.oclsCall(self.__handle, c, params.cast())
        if len(handles) is 1:
            if handles[0] is None:
                return None
            return Data(None, self.__domain, handle=handles[0])

        return [Data(None, self.__domain, handle=handle) for handle in handles]

class IntegratorFunction(CallableFunction):
    def __init__(self, framework, domain, src, function):
        if isinstance(src, str):
            src = framework.loadFromString(src)
        CallableFunction.__init__(self, c_api.oclsCreateIntegratorFunction(framework._Framework__ctx, domain._Domain__d, src, function),domain)

class FluxFunction(CallableFunction):
    def __init__(self, framework, domain, src, function):
        if isinstance(src, str):
            src = framework.loadFromString(src)
        CallableFunction.__init__(self, c_api.oclsCreateFluxFunction(framework._Framework__ctx, domain._Domain__d, src, function),domain)

class BoundaryFunction(CallableFunction):
    def __init__(self, framework, domain, src, function):
        if isinstance(src, str):
            src = framework.loadFromString(src)
        CallableFunction.__init__(self, c_api.oclsCreateBoundaryFunction(framework._Framework__ctx, domain._Domain__d, src, function),domain)

class Domain:
    def __init__(self, dimensions):
        """ Returns a numpy linspace constructed from the domain
        mesh and bounds
        """
        self.__d = c_api.oclsDomain()
        self.setDimensions(dimensions)
        self.setBounds(x=[0,1],y=[0,1],z=[0,1])
        self.setMesh(x=0,y=0,z=0)
        self.setGhostCells(0)

    def setDimensions(self, dimensions):
        self.dim = dimensions
        c_api.oclsSetDimensions(self.__d, self.dim)

    def setBounds(self, **kwargs):
        if kwargs is not None:
            if 'x' in kwargs and len(kwargs['x']) == 2:
                self.x_bounds = kwargs['x']
            if 'y' in kwargs and len(kwargs['y']) == 2:
                self.y_bounds = kwargs['y']
            if 'z' in kwargs and len(kwargs['z']) == 2:
                self.z_bounds = kwargs['z']
            c_api.oclsSetBoundsXYZ(self.__d, self.x_bounds[0], self.x_bounds[1],
                                        self.y_bounds[0], self.y_bounds[1],
                                        self.z_bounds[0], self.z_bounds[1])

    def setMesh(self, **kwargs):
        """Returns a numpy linspace constructed from the domain
        mesh and bounds
        """
        if kwargs is not None:
            if 'x' in kwargs:
                self.x_mesh = kwargs['x']
            if 'y' in kwargs:
                self.y_mesh = kwargs['y']
            if 'z' in kwargs:
                self.z_mesh = kwargs['z']
            c_api.oclsSetMeshXYZ(self.__d, self.x_mesh, self.y_mesh, self.z_mesh)

    def setGhostCells(self, ghost_cells):
        self.ghost_cells = ghost_cells
        c_api.oclsSetGhostCells(self.__d, ghost_cells)

    def dx(self):
        return c_api.oclsGetDelta(self.__d, 1)

    def dy(self):
        return c_api.oclsGetDelta(self.__d, 2)

    def dz(self):
        return c_api.oclsGetDelta(self.__d, 3)

    def toLinspace(self):
        """Returns a numpy linspace constructed from the domain
        mesh and bounds
        """
        xv = numpy.linspace(self.x_bounds[0], self.x_bounds[1], self.x_mesh)
        if self.dim == 1:
            return xv
        yv = numpy.linspace(self.y_bounds[0], self.y_bounds[1], self.y_mesh)
        if self.dim == 2:
            return xv, yv
        zv = numpy.linspace(self.z_bounds[0], self.z_bounds[1], self.z_mesh)
        return xv, yv, zv
