# Copyright (C) 2016-2019 by the multiphenics authors
#
# This file is part of multiphenics.
#
# multiphenics is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# multiphenics is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with multiphenics. If not, see <http://www.gnu.org/licenses/>.
#

from numpy import isclose, where
from petsc4py import PETSc
from ufl import *
from dolfin import *
from dolfin.cpp.mesh import GhostMode
from dolfin.fem import assemble_scalar
from dolfin.plotting import plot
import matplotlib.pyplot as plt
from multiphenics import *
from multiphenics.io import XDMFFile

r"""
In this tutorial we solve the optimal control problem

min J(y, u) = 1/2 \int_{\Gamma_2} (y - y_d)^2 ds + \alpha/2 \int_{\Gamma_2} u^2 ds
s.t.
      - \Delta y = f       in \Omega
    \partial_n y = 0       on \Gamma_1
    \partial_n y = u       on \Gamma_2
    \partial_n y = 0       on \Gamma_3
               y = 0       on \Gamma_4
             
where
    \Omega                      unit square
    u \in L^2(\Gamma_2)         control variable
    y \in H^1_0(\Omega)         state variable
    \alpha > 0                  penalization parameter
    y_d = 1                     desired state
    f                           forcing term
    
using an adjoint formulation solved by a one shot approach.
Note that this is an alternative, to Lagrange multipliers, weak imposition
of y = y_d on \Gamma_2.
"""

# MESH #
# Mesh
mesh = XDMFFile(MPI.comm_world, "data/square.xdmf").read_mesh(GhostMode.none)
boundaries = XDMFFile(MPI.comm_world, "data/square_facet_region.xdmf").read_mf_size_t(mesh)
boundaries_4 = where(boundaries.values == 4)[0]
# Neumann boundary
left = XDMFFile(MPI.comm_world, "data/square_restriction_boundary_2.rtc.xdmf").read_mesh_restriction(mesh)

# FUNCTION SPACES #
Y = FunctionSpace(mesh, ("Lagrange", 2))
U = FunctionSpace(mesh, ("Lagrange", 2))
Q = Y
W = BlockFunctionSpace([Y, U, Q], restrict=[None, left, None])

# PROBLEM DATA #
alpha = 1.e-5
y_d = 1.
x = SpatialCoordinate(mesh)
f = 10*sin(2*pi*x[0])*sin(2*pi*x[1])
def zero_eval(values, x):
    values[:] = 0.0
bc0 = interpolate(zero_eval, W.sub(0))

# TRIAL/TEST FUNCTIONS #
yup = BlockTrialFunction(W)
(y, u, p) = block_split(yup)
zvq = BlockTestFunction(W)
(z, v, q) = block_split(zvq)

# MEASURES #
ds = Measure("ds")(subdomain_data=boundaries)

# OPTIMALITY CONDITIONS #
a = [[y*z*ds(2)                 , 0              , inner(grad(p), grad(z))*dx],
     [0                         , alpha*u*v*ds(2), - p*v*ds(2)               ],
     [inner(grad(y), grad(q))*dx, - u*q*ds(2)    , 0                         ]]
f =  [y_d*z*ds(2),
      0          ,
      f*q*dx      ]
bc = BlockDirichletBC([[DirichletBC(W.sub(0), bc0, boundaries_4)],
                       [],
                       [DirichletBC(W.sub(2), bc0, boundaries_4)]])

# SOLUTION #
yup = BlockFunction(W)
(y, u, p) = block_split(yup)

# FUNCTIONAL #
J = 0.5*inner(y - y_d, y - y_d)*ds(2) + 0.5*alpha*inner(u, u)*ds(2)

# UNCONTROLLED FUNCTIONAL VALUE #
a_state = replace(a[2][0], {q: z})
f_state = replace(f[2], {q: z})
bc_state = bc[0]
solver_parameters = {"ksp_type": "preonly", "pc_type": "lu", "pc_factor_mat_solver_type": "mumps"}
solve(a_state == f_state, y, bc_state, petsc_options=solver_parameters)
y.vector.ghostUpdate(addv=PETSc.InsertMode.INSERT, mode=PETSc.ScatterMode.FORWARD)
J_uncontrolled = MPI.sum(mesh.mpi_comm(), assemble_scalar(J))
print("Uncontrolled J =", J_uncontrolled)
assert isclose(J_uncontrolled, 0.5295424)
plt.figure()
plot(y, title="uncontrolled state")
plt.show()

# OPTIMAL CONTROL #
block_solve(a, yup, f, bc, petsc_options=solver_parameters)
J_controlled = MPI.sum(mesh.mpi_comm(), assemble_scalar(J))
print("Optimal J =", J_controlled)
assert isclose(J_controlled, 8.0770392e-6)
plt.figure()
plot(y, title="state")
plt.figure()
plot(u, title="control")
plt.figure()
plot(p, title="adjoint")
plt.show()
