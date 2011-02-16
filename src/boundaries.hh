// boundaries.hh - Adjust the boundary conditions for the iPBS scheme

#ifndef _SYSPARAMS_H
#define _SYSPARAMS_H
#include "sysparams.hh"
#endif

//#include <limits>
//#include <cstdlib>

// ============================================================================

// layout for codim0 data
template <int dim>
struct P0Layout
{
  bool contains(Dune::GeometryType gt)
  {
    if ( gt.dim() == dim ) return true;
    return false;
  }
};

//===============================================================
// boundary classes - define what are inner, boundary (Neumann)
// and extended boundary (Dirichlet)elements
//===============================================================

// function defining the inner elements
template<typename GV, typename RF, typename PGMap>
class Regions
  : public Dune::PDELab::GridFunctionBase<
        Dune::PDELab::GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> >,
        Regions<GV,RF,PGMap> >
{
public:

  typedef Dune::PDELab::GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> > Traits;
  typedef Dune::PDELab::GridFunctionBase<Dune::PDELab::GridFunctionTraits<GV,RF,1,
      Dune::FieldVector<RF,1> >, Regions<GV,RF,PGMap> > BaseT;

  // constructor
  Regions(const GV& gv_, const PGMap& pg_) : gv(gv_), mapper(gv), pg(pg_) {}

  // evaluate the inner elementa
  inline void evaluate (const typename Traits::ElementType& e,
                        const typename Traits::DomainType& x,
                        typename Traits::RangeType& y) const
  {
    // retrieve element index and corresponding material index on level 0
    typename GV::template Codim<0>::EntityPointer ep(e);
    while (ep->level() != 0) ep = ep->father();
    const int ei              = mapper.map(e);
    const int physgroup_index = pg[ei];

    // evaluate physical group map and set values accordingly
    switch ( physgroup_index )
    {
      case 1  : y = 1.0;  break;
      default : y = 1.0;  break; // only one material here
    }
  }

  inline const typename Traits::GridViewType& getGridView() { return gv; }

private:

  const GV& gv;
  const Dune::MultipleCodimMultipleGeomTypeMapper<GV,P0Layout> mapper;
  const PGMap& pg;
};

// ============================================================================

/** \brief boundary grid function selecting boundary conditions
 * 0 means Neumann
 * 1 means Dirichlet
 */
template<typename GV, typename PGMap>
class BCType : public Dune::PDELab::BoundaryGridFunctionBase<
        Dune::PDELab::BoundaryGridFunctionTraits<GV,int,1,
        Dune::FieldVector<int,1> >,BCType<GV,PGMap> >
{
public:

  typedef Dune::PDELab::BoundaryGridFunctionTraits<
          GV,int,1,Dune::FieldVector<int,1> > Traits;

  //! construct from grid view
  BCType (const GV& gv_, const PGMap& pg_) : gv(gv_), pg(pg_) {}

  //! return bc type at point on intersection
  template<typename I>
  inline void evaluate (I& i, const typename Traits::DomainType& xlocal,
                        typename Traits::RangeType& y) const
  {
    // use with global ccordinates
    const int dim = Traits::GridViewType::Grid::dimension;
    typedef typename Traits::GridViewType::Grid::ctype ctype;
    Dune::FieldVector<ctype,dim> x = i.geometry().global(xlocal);
    
    // outer borders should be set to Zero Dirichlet BC (Open Boundary)
    if (x.two_norm() < 4.7)  y = 0; // Neumann in inner region
    else
	    y=1;
    return;
  }

  //! get a reference to the grid view
  inline const GV& getGridView () {return gv;}

private:

  const GV&    gv;
  const PGMap& pg;
};

// ============================================================================

// This sets the initial potential at the particle's surface and zero Dirichlet at the domain boundaries

template<typename GV, typename RF, typename PGMap>
class BCExtension_init
  : public Dune::PDELab::GridFunctionBase<Dune::PDELab::
           GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> >,
           BCExtension_init<GV,RF,PGMap> >
{
public :

  typedef Dune::PDELab::GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> > Traits;
  typedef typename GV::IntersectionIterator IntersectionIterator;
  typedef Dune::PDELab::DiscreteGridFunction<GFS,U> DGF;

  //! construct from grid view
  BCExtension_init(const GV& gv_, const PGMap& pg_) : gv(gv_), mapper(gv), pg(pg_) {}

  //! evaluate extended function on element
  inline void evaluate (const typename Traits::ElementType& e,
                        const typename Traits::DomainType& xlocal,
                        typename Traits::RangeType& y) const
  {
    // evaluate with global cordinates
    const int dim = Traits::GridViewType::Grid::dimension;
    typedef typename Traits::GridViewType::Grid::ctype ctype;
    Dune::FieldVector<ctype,dim> x = e.geometry().global(xlocal);

    /*if (x.two_norm() < 1.0+2E-1)
    {
	//y = sysParams.get_phi_init();	// set particles potential
	//y = double(rand())/(32676.0*3267.0)/5.0+0.1;
	y=5.0;
	// std::cout << "Value: " << y << std::endl;
	return;
    }
    else*/
      y = 0.0;				// set domain boundary
    return;
  }

  //! get a reference to the grid view
  inline const GV& getGridView() { return gv; }

private :

  const GV& gv;
  const Dune::MultipleCodimMultipleGeomTypeMapper<GV,P0Layout> mapper;
  const PGMap& pg;
};

// ============================================================================

// This sets the potential at the particle's surface and zero Dirichlet at the domain boundaries during iteration
/*
template<typename GV, typename RF, typename PGMap>
class BCExtension_iterate
  : public Dune::PDELab::GridFunctionBase<Dune::PDELab::
           GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> >,
           BCExtension_iterate<GV,RF,PGMap> >
{
public :

  typedef Dune::PDELab::GridFunctionTraits<GV,RF,1,Dune::FieldVector<RF,1> > Traits;
  typedef typename GV::IntersectionIterator IntersectionIterator;
  typedef Dune::PDELab::DiscreteGridFunction<GFS,U> DGF;

  //! construct from grid view
  BCExtension_iterate(const GV& gv_, const PGMap& pg_, const DGF &udgf_) : gv(gv_), mapper(gv), pg(pg_), udgf(udgf_) {}

  //! evaluate extended function on element
  inline void evaluate (const typename Traits::ElementType& e,
                        const typename Traits::DomainType& xlocal,
                        typename Traits::RangeType& y) const
  {
    // evaluate with global ccordinates
    const int dim = Traits::GridViewType::Grid::dimension;
    typedef typename Traits::GridViewType::Grid::ctype ctype;
    Dune::FieldVector<ctype,dim> x = e.geometry().global(xlocal);
    typename Traits::RangeType y_old;	// store potential at actual element for SOR step
    udgf.evaluate(e,e.geometry().local(e.geometry().center()),y_old);
    //double store = y_old;
    Dune::FieldVector <ctype,dim> r = e.geometry().center();
    r *= sysParams.get_radius();		// scale all vectors with the length-scale of our systems

    if (x.two_norm() < 1.0+2E-1)
    //if (e.hasBoundaryIntersections() == true && x.two_norm() < 2.0)
    {
    	// std::cout << "x = " << x.vec_access(0) << "\ty = " << x.vec_access(1) << "\ty_old: " << y_old << std::endl;
        // Take a SOR step towards the correct solution (see paper)
	// calculate B.C. according to eq. 2 (paper)
	y = 0;
	typedef typename GV::template Codim<0>::Iterator LeafIterator;
	for (LeafIterator it = gv.template begin<0>(); it != gv.template end<0>(); ++it)
	{
	  typename Traits::RangeType value;
	  Dune::FieldVector<ctype,dim> r_prime = it->geometry().center();
          r_prime *= sysParams.get_radius();	// scale all vectors with the length-scale of our systems
	  if (r != r_prime)	// make sure we have no zero division
	  {
	    udgf.evaluate(*it,it->geometry().local(it->geometry().center()),value);
	    Dune::FieldVector<ctype,dim> dist = r - r_prime;
    	    //std::cout << "x = " << x.vec_access(0) << "\ty = " << x.vec_access(1) << "\tValue: " << value << std::endl;
  	    y += std::sinh(value) / dist.two_norm() * it->geometry().volume();
    	    //std::cout << "sinh: " << y << std::endl;
	  }
	}
    	//std::cout << "\ty nach int: " << y << std::endl;
	y *= sysParams.get_lambda2i() / sysParams.get_bjerrum() / (4.0*sysParams.pi);
	// Currently we use a fixed point charge at origin (sphere case)
	y += sysParams.get_bjerrum() * sysParams.get_charge() / (sysParams.get_epsilon() * r.two_norm());
    	// std::cout << "\ty_neu: " << y;
	// SOR step
	 y = sysParams.get_alpha() * y + (1.0 - sysParams.get_alpha()) * y_old;
	// y = 0.7 * y + (1.0 - 0.7) * y_old;
	//std::cout << std::endl <<"y nach SOR: " <<  y << "\ty_old = " << y_old << "\t|y-y_old|: " << double(abs(double(y)-double(y_old))) << std::endl;
	// Calculate error
	if (fabs(y - y_old) > sysParams.error_cut)
	{
           double error = fabs(2.0*(double(y-y_old)/double(y+y_old)));
	if (fabs(y_old) > 1E-3)
	//	std::cout << std::endl << "An x = " << x.vec_access(0) << " y = " << x.vec_access(1) <<  ":" << std::endl <<"y nach SOR: " <<  y << "\ty_old = " << y_old << "\terror: " << error << std::endl;
	  //sysParams.add_error(2.0*(double(y-y_old)/double(y+y_old));
	  sysParams.add_error(error);
	  //sysParams.add_error(y-y_old);
	}
	return;
    }
    else if (x.two_norm() > 4.9)
      y = 0.0;			// Dirichlet B.C. for domain
    return;
  }

  //! get a reference to the grid view
  inline const GV& getGridView() { return gv; }

private :

  const GV& gv;
  const Dune::MultipleCodimMultipleGeomTypeMapper<GV,P0Layout> mapper;
  const PGMap& pg;
  const DGF& udgf;
};
*/
// ============================================================================

// function for defining radiation and Neumann boundary conditions

template<typename GV, typename RF, typename PGMap>
class BoundaryFlux
  : public Dune::PDELab::BoundaryGridFunctionBase<
           Dune::PDELab::BoundaryGridFunctionTraits<GV,RF,1,
           Dune::FieldVector<RF,1> >, BoundaryFlux<GV,RF,PGMap> >
{
public:

  typedef Dune::PDELab::BoundaryGridFunctionTraits<
          GV,RF,1,Dune::FieldVector<RF,1> > Traits;
  typedef typename Traits::GridViewType::Grid::ctype ctype;

  // constructor
  BoundaryFlux(const GV& gv_, const PGMap& pg_) : gv(gv_), pg(pg_) {}

  // evaluate flux boundary condition
  template<typename I, typename E>
  inline void evaluate(I& i, E& e,
                       typename Traits::RangeType& y, const DGF& udgf, Dune::FieldVector<RF,dim> gradu) const
  {
    
    // Get the vector of the actual intersection
    Dune::FieldVector<ctype,dim> r = i.geometry().global(e->position());
    // Scale the vectors with particle's radius
    r *= sysParams.get_radius();
    // Get the unit normal vector of the surface element
    Dune::FieldVector<ctype,dim> unitNormal = i.centerUnitOuterNormal();
    
    typename Traits::RangeType phi_old;	// store potential at actual element for SOR step
    typename Traits::DomainType xlocal;
    //Dune::FieldVector<ctype,dim> surfaceElementCenter = i.geometryInInside().center();
    udgf.evaluate(*i.inside(),i.geometry().global(e->position()),phi_old);
    //udgf.evaluate(*i.inside(), xlocal, phi_old);
    //std::cout << "Evaluated potential at " << i.geometry().global(e->position()) << "\tvalue: " << phi_old << std::endl;
    
    typedef typename GV::template Codim<0>::Iterator LeafIterator; // Iterator type for integrationIterator

    y = 0.0;
    
    // Integration Loop

     for (LeafIterator integrationIterator = gv.template begin<0>(); integrationIterator != gv.template end<0>(); ++integrationIterator)
     {
      typename Traits::RangeType value;
      Dune::FieldVector<ctype,dim> r_prime = integrationIterator->geometry().center();
      r_prime *= sysParams.get_radius();	// scale all vectors with the particle's radius
      udgf.evaluate(*integrationIterator,r_prime,value);
      Dune::FieldVector<ctype,dim> dist = r - r_prime;
      //std::cout << "x = " << x.vec_access(0) << "\ty = " << x.vec_access(1) << "\tValue: " << value << std::endl;
      double volume = integrationIterator->geometry().volume();
      y += std::sinh(value) / (dist.two_norm() *dist.two_norm() * dist.two_norm()) * volume * (dist * unitNormal);
      //std::cout << "sinh: " << test << std::endl;
     }
    y *= sysParams.get_lambda2i() / sysParams.get_bjerrum() / (4.0*sysParams.pi);
    
    
    // Calculate Q/R^3 * [\vec(r) * \vec(n)]
    y += sysParams.get_sigma_sphere() / (sysParams.get_radius()*sysParams.get_radius()* sysParams.get_radius())* (r * unitNormal);
    
    // std::cout << "\tsigma = " << y << "\t-grad*n = " << -1.0 * (grad * i.unitOuterNormal(e->position())) << "\tgrad = " << grad 
    // << "\tunitOuterNormal = " << i.unitOuterNormal(e->position()) << "\tcenterUnitOuterNormal = " << unitNormal << std::endl;
    
    //std::cout << std::endl << "An r[0] = " << r.vec_access(0) << "\tr[1] = " << r.vec_access(1) <<  ":\t y = " << y << std::endl;
    
    double yOld = - 1.0 * (gradu * unitNormal);
    y = sysParams.get_alpha() * y + (1.0 - sysParams.get_alpha()) * yOld;
    double error = fabs(2.0*(double(y-yOld)/double(y+yOld)));
    sysParams.add_error(error);

    //std::cout <<"\ty nach SOR: " <<  y << "\ty_old = " << yOld << "\terror: " << error << std::endl;
    
    return;
  }

private:

  const GV&    gv;
  const PGMap& pg;
};