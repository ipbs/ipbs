/** \file

    \brief IPBS - An Iterative Poisson Boltzmann implementation using DUNE

    Here goes some explanation on what is done :-)
    \todo { Doc Me ! }   
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// std includes
//#include<math.h>
//#include<iostream>
//#include<vector>
//#include<string>

// global DUNE includes
#include<dune/common/mpihelper.hh>
#include<dune/common/collectivecommunication.hh>
#include<dune/common/exceptions.hh>
#include<dune/common/timer.hh>

//TODO: sort out
//#include <dune/pdelab/adaptivity/adapt.hh>
//#include <dune/grid/common/gridenums.hh>
//#include <dune/common/dynmatrix.hh>
//#include<dune/pdelab/gridfunctionspace/constraints.hh>
//#include<dune/pdelab/gridfunctionspace/gridfunctionspaceutilities.hh>
//#include<dune/pdelab/gridfunctionspace/genericdatahandle.hh>
//#include<dune/pdelab/finiteelementmap/p1fem.hh>	// P1 in 1,2,3 dimensions

/* include grid IO */
#include<dune/grid/io/file/gmshreader.hh>
#include <dune/grid/utility/gridtype.hh>

// pdelab includes
#include<dune/pdelab/finiteelementmap/conformingconstraints.hh>
#include<dune/pdelab/gridfunctionspace/gridfunctionspace.hh>
#include<dune/pdelab/gridfunctionspace/interpolate.hh>
#include<dune/pdelab/backend/istlvectorbackend.hh>
#include<dune/pdelab/backend/istlmatrixbackend.hh>
#include<dune/pdelab/backend/istlsolverbackend.hh>


// global typedefs
typedef double Real;

#include "sysparams.hh"
#include "boundary.hh"
#include "parser.hh"
#include "ipbs_Pk.hh"

// global access to particles
std::vector<Boundary*> boundary;
SysParams sysParams;

//===============================================================
// Main programm
//===============================================================
int main(int argc, char** argv)
{
 try{
  // Initialize Mpi
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
   if(Dune::MPIHelper::isFake)
    std::cout<< "This is a sequential program!" << std::endl;
  else
  {
    if(helper.rank()==0)
    {
       std::cout << "Hello World! This is iPBS." << std::endl;
       std::cout << "parallel run on " << helper.size() << " process(es)" << std::endl;
    }
  }
  
  // check arguments
  if (argc!=2)
  {
    if (helper.rank()==0)
    {
	std::cout << "usage: ./iPBS <configuration file>" << std::endl;
	return 1;
    }
  }
  
  // Parse configuration file.
  std::string config_file(argv[1]);
  parser(config_file);

  
//===============================================================
// Setup problem
//===============================================================
  
  
  // <<<1>>> Setup the problem from mesh file
  
  // define vectors to store boundary and element mapping
  std::vector<int> boundaryIndexToEntity;
  std::vector<int> elementIndexToEntity;
  
  typedef Dune::GridSelector::GridType GridType;
  Dune::GridFactory<GridType> factory;

 
  if(helper.rank() == 0)
  {
    // read a gmsh file
    Dune::GmshReader<GridType> gmshreader;
    gmshreader.read(factory, sysParams.get_meshfile(), boundaryIndexToEntity, elementIndexToEntity, true, true);
  }

  // MPIHelper ensures that this works for the sequential case
  Dune::CollectiveCommunication<Dune::MPIHelper::MPICommunicator> colCom(helper.getCommunicator());
 
  // Communicate boundary vector
  int size = boundaryIndexToEntity.size();
  colCom.broadcast (&size, 1, 0);
  if (helper.rank() > 0)
    boundaryIndexToEntity.reserve(size);
  colCom.broadcast(&boundaryIndexToEntity[0],size,0);
  
  // create the grid
  GridType* grid = factory.createGrid();
 
  // refine grid
  if(helper.rank()==0) {
    std::cout << "Using " << sysParams.get_refinement() << " global refinement steps and" << std::endl;
    std::cout << sysParams.get_refinementSteps() << " adaptive refinement steps with "
      << sysParams.get_refinementFraction() << " percent refinement." << std::endl;
  }


  // Load balance the parallel grid
  // grid->globalRefine(sysParams.get_refinement());
 
  // Load balance the parallel grid
  std::cout << "Grid has been modified by load balancing: " << grid->loadBalance() << std::endl;

 // Call problem driver
 ipbs_Pk<GridType, 2>(grid, elementIndexToEntity, boundaryIndexToEntity, helper);
  
 // done
 return 0;
 }
 catch (Dune::Exception &e){
  std::cerr << "Dune reported error: " << e << std::endl;
 }
 catch (...){
  std::cerr << "Unknown exception thrown!" << std::endl;
 }
} 

// ============================================================================
