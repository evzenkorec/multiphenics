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

#ifndef __BLOCK_FUNCTION_SPACE_H
#define __BLOCK_FUNCTION_SPACE_H

#include <dolfin/function/FunctionSpace.h>
#include <multiphenics/fem/BlockDofMap.h>

namespace multiphenics
{
  namespace function
  {
    /// This class represents a finite element function space defined by
    /// a mesh, a vector of finite elements, and a vector of local-to-global mapping of the
    /// degrees of freedom (dofmap).

    class BlockFunctionSpace
    {
    public:
    
      /// Create a block function space from a list of existing function spaces (on the same mesh)
      /// @param[in] function_spaces List of existing function spaces.
      explicit BlockFunctionSpace(std::vector<std::shared_ptr<const dolfin::function::FunctionSpace>> function_spaces);
      
      /// Create a block function space from a list of existing function spaces (on the same mesh),
      /// but keeping only a part
      /// @param[in] function_spaces List of existing function spaces
      /// @param[in] restrictions Vector (over blocks) of vector (over space dimensions) representing the restrictions
      BlockFunctionSpace(std::vector<std::shared_ptr<const dolfin::function::FunctionSpace>> function_spaces,
                         std::vector<std::vector<std::shared_ptr<const dolfin::mesh::MeshFunction<std::size_t>>>> restrictions);

      /// Create a block function space for given mesh, vector of elements and vector of dofmaps
      /// @param[in] mesh The mesh
      /// @param[in] elements The elements
      /// @param[in] dofmaps The dofmaps
      BlockFunctionSpace(std::shared_ptr<const dolfin::mesh::Mesh> mesh,
                         std::vector<std::shared_ptr<const dolfin::fem::FiniteElement>> elements,
                         std::vector<std::shared_ptr<const dolfin::fem::DofMap>> dofmaps);
                    
      /// Create a block function space for given mesh, vector of elements and vector of dofmaps
      /// but keeping only a part 
      /// @param[in] mesh The mesh
      /// @param[in] elements The elements
      /// @param[in] dofmaps The dofmaps
      /// @param[in] restrictions Vector (over blocks) of vector (over space dimensions) representing the restrictions
      BlockFunctionSpace(std::shared_ptr<const dolfin::mesh::Mesh> mesh,
                         std::vector<std::shared_ptr<const dolfin::fem::FiniteElement>> elements,
                         std::vector<std::shared_ptr<const dolfin::fem::DofMap>> dofmaps,
                         std::vector<std::vector<std::shared_ptr<const dolfin::mesh::MeshFunction<std::size_t>>>> restrictions);

      /// Copy constructor (deleted)
      BlockFunctionSpace(const BlockFunctionSpace& V) = delete;
      
      /// Move constructor
      BlockFunctionSpace(BlockFunctionSpace&& V) = default;

      /// Destructor
      virtual ~BlockFunctionSpace() = default;
      
    private:
                                           
      // Initialize _elements and _dofmaps from function spaces
      void _init_mesh_and_elements_and_dofmaps_from_function_spaces();
      
      // Initialize _function_spaces from elements and dofmaps
      void _init_function_spaces_from_elements_and_dofmaps();
                                 
      // Initialize _block_dofmap from dofmaps and restrictions
      void _init_block_dofmap_from_dofmaps_and_restrictions();

    public:
      
      /// Assignment operator (deleted)
      BlockFunctionSpace& operator=(const BlockFunctionSpace& V) = delete;
      
      /// Move assignment operator
      BlockFunctionSpace& operator=(BlockFunctionSpace&& V) = default;

      /// Equality operator
      /// @param[in] V Another block function space.
      bool operator== (const BlockFunctionSpace& V) const;

      /// Inequality operator
      /// @param[in] V Another block function space.
      bool operator!= (const BlockFunctionSpace& V) const;

      /// Return dimension of function space
      /// @return The dimension of the block function space.
      std::int64_t dim() const;

      /// Extract subspace for component, *neglecting* restrictions
      /// @param[in] i Index of the subspace.
      /// @return The subspace.
      std::shared_ptr<const dolfin::function::FunctionSpace> operator[] (std::size_t i) const;

      /// Extract subspace for component, *neglecting* restrictions
      /// @param[in] i Index of the subspace.
      /// @return The subspace.
      std::shared_ptr<const dolfin::function::FunctionSpace> sub(std::size_t component) const;

      /// Extract block subspace for component, possibly considering restrictions
      /// @param[in] component Indices of the subspaces.
      /// @return The block subspace.
      std::shared_ptr<BlockFunctionSpace>
      extract_block_sub_space(const std::vector<std::size_t>& component, bool with_restrictions=true) const;

      /// Check whether V is subspace of this, or this itself
      ///
      /// *Arguments*
      ///     V (_FunctionSpace_)
      ///         The space to be tested for inclusion.
      ///
      /// *Returns*
      ///     bool
      ///         True if V is contained or equal to this.
      bool contains(const BlockFunctionSpace& V) const;

      /// Return component w.r.t. to root superspace, i.e.
      ///   W.sub(1).sub(0) == [1, 0].
      ///
      /// *Returns*
      ///     std::vector<std::size_t>
      ///         The component (w.r.t to root superspace).
      std::vector<std::size_t> component() const;

      /// Return informal string representation (pretty-print)
      ///
      /// *Arguments*
      ///     verbose (bool)
      ///         Flag to turn on additional output.
      ///
      /// *Returns*
      ///     std::string
      ///         An informal representation of the function space.
      std::string str(bool verbose) const;
      
      /// Tabulate the coordinates of all dofs on this process. This
      /// function is typically used by preconditioners that require the
      /// spatial coordinates of dofs, for example for re-partitioning or
      /// nullspace computations.
      ///
      /// *Returns*
      ///     std::vector<double>
      ///         The dof coordinates (x0, y0, x1, y1, . . .)
      dolfin::EigenRowArrayXXd tabulate_dof_coordinates() const;
      
      // The mesh
      std::shared_ptr<const dolfin::mesh::Mesh> mesh;

      // The finite elements
      std::vector<std::shared_ptr<const dolfin::fem::FiniteElement>> elements;

      // The dofmaps
      std::vector<std::shared_ptr<const dolfin::fem::DofMap>> dofmaps;
      
      // The block dofmap
      std::shared_ptr<multiphenics::fem::BlockDofMap> block_dofmap;
      
      // The subspaces
      std::vector<std::shared_ptr<const dolfin::function::FunctionSpace>> function_spaces;
      
      // The restrictions
      std::vector<std::vector<std::shared_ptr<const dolfin::mesh::MeshFunction<std::size_t>>>> restrictions;
      
    private:
      // The component w.r.t. to root space
      std::vector<std::size_t> _component;

      // The identifier of root space
      std::size_t _root_space_id;

      // Cache of block subspaces
      typedef std::map<std::vector<std::size_t>,
                       std::shared_ptr<BlockFunctionSpace>> BlockSubpsacesType;
      mutable BlockSubpsacesType _block_subspaces__with_restrictions;
      mutable BlockSubpsacesType _block_subspaces__without_restrictions;

    };
  }
}

#endif
