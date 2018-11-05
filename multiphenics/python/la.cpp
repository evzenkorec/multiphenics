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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <multiphenics/python/mpi_casters.h> // TODO remove local copy of DOLFIN's pybind11 files
#include <multiphenics/python/petsc_casters.h> // TODO remove local copy of DOLFIN's pybind11 files

namespace py = pybind11;

namespace multiphenics_wrappers
{
  void la(py::module& m)
  {
    // multiphenics::la::BlockMATLABExport
    py::class_<multiphenics::la::BlockMATLABExport>
      (m, "BlockMATLABExport", "multiphenics BlockMATLABExport object")
      .def_static("export_", (void (*)(const dolfin::la::PETScMatrix&, std::string))
                  &multiphenics::la::BlockMATLABExport::export_)
      .def_static("export_", (void (*)(const dolfin::la::PETScVector&, std::string))
                  &multiphenics::la::BlockMATLABExport::export_);
      
    // multiphenics::la::BlockInsertMode
    py::enum_<multiphenics::la::BlockInsertMode>
      (m, "BlockInsertMode", "multiphenics BlockInsertMode enum")
      .value("INSERT_VALUES", multiphenics::la::BlockInsertMode::INSERT_VALUES)
      .value("ADD_VALUES", multiphenics::la::BlockInsertMode::ADD_VALUES);
      
    #ifdef HAS_PETSC
    // multiphenics::la::BlockPETScVector
    py::class_<multiphenics::la::BlockPETScVector, std::shared_ptr<multiphenics::la::BlockPETScVector>, dolfin::PETScVector>
      (m, "BlockPETScVector", "multiphenics BlockPETScVector object")
      .def(py::init<>())
      .def(py::init<Vec>())
      .def("attach_block_dof_map", &multiphenics::la::BlockPETScVector::attach_block_dof_map)
      .def("get_block_dof_map", &multiphenics::la::BlockPETScVector::get_block_dof_map)
      .def("has_block_dof_map", &multiphenics::la::BlockPETScVector::has_block_dof_map);
      
    // multiphenics::la::BlockPETScMatrix
    py::class_<multiphenics::la::BlockPETScMatrix, std::shared_ptr<multiphenics::la::BlockPETScMatrix>, dolfin::PETScMatrix>
      (m, "BlockPETScMatrix", "multiphenics BlockPETScMatrix object")
      .def(py::init<>())
      .def(py::init<Mat>())
      .def("attach_block_dof_map", &multiphenics::la::BlockPETScMatrix::attach_block_dof_map)
      .def("get_block_dof_map", &multiphenics::la::BlockPETScMatrix::get_block_dof_map)
      .def("has_block_dof_map", &multiphenics::la::BlockPETScMatrix::has_block_dof_map);
      
    // multiphenics::la::BlockPETScSubVector
    py::class_<multiphenics::la::BlockPETScSubVector, std::shared_ptr<multiphenics::la::BlockPETScSubVector>, dolfin::PETScVector>
      (m, "BlockPETScSubVector", "multiphenics BlockPETScSubVector object");
      
    // multiphenics::la::BlockPETScSubMatrix
    py::class_<multiphenics::la::BlockPETScSubMatrix, std::shared_ptr<multiphenics::la::BlockPETScSubMatrix>, dolfin::PETScMatrix>
      (m, "BlockPETScSubMatrix", "multiphenics BlockPETScSubMatrix object");
    #endif
      
    #ifdef HAS_SLEPC
    // dolfin::la::CondensedSLEPcEigenSolver
    py::class_<dolfin::la::CondensedSLEPcEigenSolver, std::shared_ptr<dolfin::la::CondensedSLEPcEigenSolver>, dolfin::SLEPcEigenSolver>
      (m, "CondensedSLEPcEigenSolver", "multiphenics CondensedSLEPcEigenSolver object")
      .def(py::init<std::shared_ptr<const dolfin::PETScMatrix>,
                    std::vector<std::shared_ptr<const dolfin::DirichletBC>>>())
      .def(py::init<std::shared_ptr<const dolfin::PETScMatrix>, std::shared_ptr<const dolfin::PETScMatrix>,
                    std::vector<std::shared_ptr<const dolfin::DirichletBC>>>())
      .def("get_eigenpair", [](dolfin::la::CondensedSLEPcEigenSolver& self, dolfin::Function& r_fun, dolfin::Function& c_fun, std::size_t i)
           {
             double lr, lc;
             dolfin::PETScVector r, c; // cannot use r_fun and c_fun vectors due to different ghosting
             self.get_eigenpair(lr, lc, r, c, i);
             std::vector<double> r_local;
             r.get_local(r_local);
             r_fun.vector()->set_local(r_local);
             r_fun.vector()->apply("insert");
             std::vector<double> c_local;
             c.get_local(c_local);
             c_fun.vector()->set_local(c_local);
             c_fun.vector()->apply("insert");
             return py::make_tuple(lr, lc, r_fun, c_fun);
           });
    
    // multiphenics::la::CondensedBlockSLEPcEigenSolver
    py::class_<multiphenics::la::CondensedBlockSLEPcEigenSolver, std::shared_ptr<multiphenics::la::CondensedBlockSLEPcEigenSolver>, dolfin::la::CondensedSLEPcEigenSolver>
      (m, "CondensedBlockSLEPcEigenSolver", "multiphenics CondensedBlockSLEPcEigenSolver object")
      .def(py::init<std::shared_ptr<const multiphenics::la::BlockPETScMatrix>,
                    std::shared_ptr<const multiphenics::fem::BlockDirichletBC>>())
      .def(py::init<std::shared_ptr<const multiphenics::la::BlockPETScMatrix>, std::shared_ptr<const multiphenics::la::BlockPETScMatrix>,
                    std::shared_ptr<const multiphenics::fem::BlockDirichletBC>>())
      .def("get_eigenpair", [](multiphenics::la::CondensedBlockSLEPcEigenSolver& self, multiphenics::function::BlockFunction& r_fun, multiphenics::function::BlockFunction& c_fun, std::size_t i)
           {
             double lr, lc;
             multiphenics::la::BlockPETScVector r, c; // cannot use r_fun and c_fun block vectors due to different ghosting
             self.get_eigenpair(lr, lc, r, c, i);
             std::vector<double> r_local;
             r.get_local(r_local);
             r_fun.block_vector()->set_local(r_local);
             r_fun.block_vector()->apply("insert");
             r_fun.apply("to subfunctions");
             std::vector<double> c_local;
             c.get_local(c_local);
             c_fun.block_vector()->set_local(c_local);
             c_fun.block_vector()->apply("insert");
             c_fun.apply("to subfunctions");
             return py::make_tuple(lr, lc, r_fun, c_fun);
           });
    #endif
  }
}
