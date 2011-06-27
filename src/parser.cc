/** \file
    
    \brief Read in IPBS configuration parameters from a config file 
    
    \todo Doc me!
*/

#include<string>
#include<dune/common/parametertree.hh>
#include<dune/common/parametertreeparser.hh>
#ifndef _SYSPARAMS_H
#define _SYSPARAMS_H
#include "sysparams.hh"
#endif
#ifndef _BOUNDARY_H
#define _BOUNDARY_H
#include "boundary.hh"
#endif

void parser(std::string config_file)
{
  Dune::ParameterTree configuration;
  Dune::ParameterTreeParser parser;
  
  try{
      parser.readINITree( config_file, configuration );
  }
  catch(...){
      std::cerr << "Could not read config file \"" 
                << config_file << "\"!" << std::endl;
      exit(1);
  }

  // Mesh file must be specified
  sysParams.set_meshfile(configuration.get<std::string>("mesh.filename"));

  // default values
  const double alpha_sor = 0.7;
  const int level = 0;
  const int verbose = 4;
  const double lambda = 1.0;
  const double bjerrum = 0.7;

  // set the symmetry of the system
  sysParams.set_symmetry(configuration.get<double>("mesh.symmetry"));
  sysParams.set_boxLength(configuration.get<double>("mesh.boxLength",0.0));
  
  // Parse other options
  sysParams.set_alpha(configuration.get<double>("solver.alpha_sor",alpha_sor));
  sysParams.set_refinement(configuration.get<int>("mesh.global_refinement_level",level));
  sysParams.set_refinementFraction(configuration.get<double>("mesh.adaptive_refinement_fraction",0));
  sysParams.set_refinementSteps(configuration.get<int>("mesh.adaptive_refinement_steps",1));
  sysParams.set_bjerrum(configuration.get<double>("system.bjerrum",bjerrum));
  sysParams.set_lambda(configuration.get<double>("system.lambda",lambda));
  sysParams.set_tolerance(configuration.get<double>("solver.tolerance"));
  sysParams.set_verbose(configuration.get<int>("system.verbose",verbose));
  sysParams.set_salt(configuration.get<int>("system.salt"));

  // Create particles
  int n_particle = configuration.get<int>("system.NPart");
  sysParams.set_npart(n_particle);
  extern std::vector<Boundary*> boundary;
  for (int i = 0; i < n_particle; i++)
    boundary.push_back(new Boundary());

  for(int i = 0; i < n_particle; i++)
  {
    std::string p_name = "boundary_";
    std::ostringstream s;
    s << p_name << i+2; // we don't need to set 0 and 1
                        // remember that vector starts with 0, so access the boundaries with -2
                        // TODO find a clever way!
    p_name = s.str();
    boundary[i]->set_charge_density(configuration.get<double>(p_name+".charge_density"));
  }
}
