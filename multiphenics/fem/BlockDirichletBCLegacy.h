// Copyright (C) 2016-2019 by the multiphenics authors
//
// This file is part of multiphenics.
//
// multiphenics is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// multiphenics is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with multiphenics. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __BLOCK_DIRICHLET_BC_LEGACY_H
#define __BLOCK_DIRICHLET_BC_LEGACY_H

#include <multiphenics/fem/BlockDirichletBC.h>
#include <dolfin/la/PETScMatrix.h>
#include <dolfin/la/PETScVector.h>

namespace multiphenics
{
  namespace fem
  {
    class BlockDirichletBCLegacy
    {
    public:
      /// Apply block boundary conditions to a matrix
      static void apply(const BlockDirichletBC& bcs,
                        dolfin::la::PETScMatrix& A,
                        PetscScalar diag);

      /// Apply block boundary conditions to a vector
      static void apply(const BlockDirichletBC& bcs,
                        dolfin::la::PETScVector& b);

      /// Apply block boundary conditions to vectors for a nonlinear problem
      static void apply(const BlockDirichletBC& bcs,
                        dolfin::la::PETScVector& b,
                        const dolfin::la::PETScVector& x);
    private:
      /// Collect common implementation for application to vector
      static void _apply(std::vector<std::shared_ptr<const dolfin::fem::DirichletBC>> bcs,
                         std::vector<PetscInt>& unrestricted_rows,
                         std::vector<PetscScalar>& unrestricted_values);
    };
  }
}

#endif
