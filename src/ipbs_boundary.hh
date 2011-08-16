/** \brief Precompute the IPBS boundary flux values.

    Here goes some explanation on what is done :-)
    \todo { Doc Me ! }   
*/

#include <gsl/gsl_sf_ellint.h>

template <class GV, class DGF, class ElemPointer,
         typename IndexLookupMap, typename BoundaryElemMapper>

void ipbs_boundary(const GV& gv, const DGF& udgf,
      const std::vector<ElemPointer> ipbsElems,
			double fluxContainer[], const int countBoundElems,
      const std::vector<int>& boundaryIndexToEntity,
      const IndexLookupMap& indexLookupMap, const BoundaryElemMapper& boundaryElemMapper)
{
  // some typedef
  const int dim = GV::dimension;
  typedef typename GV::Grid::ctype ctype;
  typedef typename GV::template Codim<0>::template Partition
          <Dune::Interior_Partition>::Iterator LeafIterator;
  typedef typename GV::IntersectionIterator IntersectionIterator;
  typedef typename DGF::Traits::RangeType RT;	// store potential during integration 
                                  						// (for calculating sinh-term)
  
  // Initialize functor for integrating coulomb flux
  CoulombFlux<ctype,dim> f;
  double dielectric_factor = 0;   // = 2 eps_in / (eps_in + eps_out)
  
  std::cout << "in ipbs_boundary, countBoundElems = " << countBoundElems << std::endl;
  // Precompute fluxes
  for(int i = 0; i < countBoundElems; i++)
  {
    // Get the unit normal vector of the surface element
    Dune::FieldVector<ctype,dim> unitNormal, r;
    for (IntersectionIterator ii = gv.ibegin(*ipbsElems[i]); ii != gv.iend(*ipbsElems[i]); ++ii)
      if (ii->boundary() == true)
      {
        if (boundaryIndexToEntity[ii->boundarySegmentIndex()] > 1)
        {
          unitNormal = ii->centerUnitOuterNormal();
          r = ii->geometry().center();  // vector of iterative surface boundary center
          dielectric_factor = boundary[boundaryIndexToEntity[ii->boundarySegmentIndex()]-2]->get_dielectric_factor();
        }
      }
    unitNormal*=-1.0;	// turn around unit vector as it is outer normal
   
    double summed_flux = 0;
    // double summed_volume_flux = 0;
	   
    for (LeafIterator it = gv.template begin<0,Dune::Interior_Partition>();
            	it!=gv.template end<0,Dune::Interior_Partition>(); ++it)
    {
      // Now we go over all elements on this processor
      // and sum up their flux contributions
      Dune::FieldVector<ctype,dim> E_field(0);
      // Dune::FieldVector<ctype,dim> surfaceElem_field(0);
      // Dune::FieldVector<ctype,dim> volumeElem_field(0);
	  
      // Get the position vector of the this element's center
      Dune::FieldVector<ctype,dim> r_prime = it->geometry().center();
      Dune::FieldVector<ctype,dim> dist = r - r_prime;
      // std::cout << "In the loop :-) \t r_prime = " << r_prime << std::endl;
	  
      // integrate sinh over all volume elements 
      double a, b, k, K, E;  // parameters for elliptic
      double volumeElem_flux, surfaceElem_flux, itself_flux;
      typedef typename GV::IntersectionIterator IntersectionIterator;
      
      // ================================================================== 
      // Integral over all volume elements
      // ================================================================== 
      
      // Evaluate the potential at the elements center
	    RT value;
      udgf.evaluate(*it,it->geometry().local(it->geometry().center()),value);
      // Calculate the flux through surface element caused by this element

      if (it->hasBoundaryIntersections() == false && it != ipbsElems[i])
      {
      // integration depends on symmetry
      switch( sysParams.get_symmetry() )
      {
        case 1:   // infinite cylinder
          {
            // Calculate the integrated sinh term for infinite geometry
            if (dist[0] > sysParams.get_boxLength()/2.0) dist[0]-=sysParams.get_boxLength();
            else if (dist[0] < -sysParams.get_boxLength()/2.0) dist[0] +=sysParams.get_boxLength();
            a = (dist[0]*dist[0] + r[1]*r[1] + r_prime[1]*r_prime[1]);
	          b = 2.0 * r[1] * r_prime[1];
            k = sqrt(2.0*b/(a+b));
            
            E_field[0] = 0.0*sqrt((a-b)/(a+b))
                        *gsl_sf_ellint_Ecomp (k, GSL_PREC_DOUBLE)
                         / sqrt((a-b)*(a-b)*(a-b))* dist[0];
            E_field[1] = 4.0*sqrt((a-b)/(a+b))
                        * ((-a * r_prime[1] + b * r[1]) * gsl_sf_ellint_Ecomp (k, GSL_PREC_DOUBLE)
                            + (a-b) * r_prime[1] * gsl_sf_ellint_Kcomp(k, GSL_PREC_DOUBLE))
                        / sqrt((a-b)*(a-b)*(a-b)) / b;
                       
            volumeElem_flux = E_field * unitNormal;
            volumeElem_flux *= -1.0 / (sysParams.get_bjerrum() * 4.0 * sysParams.pi)
                    * sysParams.get_lambda2i() * it->geometry().volume() * r_prime[1];

          }
          break;
        
        case 2:   // spherical reduced 3d symmetry
	        {
            a = dist[0]*dist[0] + r[1]*r[1] + r_prime[1]*r_prime[1] + 2.0 * r[1] * r_prime[1];
            b = 4.0 * r[1] * r_prime[1];
            // if (a > 1e-3)
            // {
            k = sqrt(b/a);
            E = gsl_sf_ellint_Ecomp (k, GSL_PREC_DOUBLE);
            K = gsl_sf_ellint_Kcomp (k, GSL_PREC_DOUBLE);
            // }
            // else
            // {
            //   E = sysParams.pi / 2.0;
            //   K = sysParams.pi / 2.0;
            // }
            E_field[0] = -2.0 * dist[0] / ((a-b)*sqrt(a)) * E;
            E_field[1] = 2.0 * ( 2.0 * r_prime[1] * a - b * r_prime[1] - b * r[1])
                           / ((a-b)*sqrt(a)*b) * E
                         + 2.0 * (-2.0 * r_prime[1] * a + 2.0 * b * r_prime[1]) /  ((a-b)*sqrt(a)*b) * K;
            volumeElem_flux = E_field * unitNormal;
            volumeElem_flux *= 1.0 / (4.0*sysParams.pi) * sysParams.get_lambda2i() * it->geometry().volume() * r_prime[1];
          }
	        break;

        case 3: // An ininite plane in 2d cartesion coordinates 
          {
            // Calculate the integrated sinh term for infinite geometry
            if (dist[0] > sysParams.get_boxLength()/2.0) dist[0]-=sysParams.get_boxLength();
            else if (dist[0] < -sysParams.get_boxLength()/2.0) dist[0] +=sysParams.get_boxLength();
           
            E_field = dist;
            E_field /= dist.two_norm() * dist.two_norm() * 2.0 * sysParams.pi;
                      
            volumeElem_flux = E_field * unitNormal;
            volumeElem_flux *= 1.0 / sysParams.get_bjerrum()
                    * sysParams.get_lambda2i() * it->geometry().volume();
          }
          break;
      }

        // Now get the counterion distribution and calculate flux
        switch (sysParams.get_salt())
        {
          case 0:
            volumeElem_flux *= std::sinh(value);
            break;
          case 1:
            volumeElem_flux *= std::exp(value); // Counterions have opposite sign!
            break;
        }
        summed_flux += volumeElem_flux;
        // summed_volume_flux += volumeElem_flux;
        //std::cout << "at elem " << r << " contrib from " << r_prime << " elem Volume flux = " << volumeElem_flux << " phi = " << value << " sinh = " << std::sinh(value) << " a = " << a << " b = " << b << " E = " << E << " K = " << K << " summed_volume_flux = " << summed_volume_flux <<" summed flux = " << summed_flux << std::endl;
      }

      // ================================================================== 
      // Integral over surface elements
      // ================================================================== 
 
      if (it->hasBoundaryIntersections() == true)
      {
        for (IntersectionIterator ii = gv.ibegin(*it); ii != gv.iend(*it); ++ii)
          {
             if(ii->boundary() == true)
             {
                if (boundaryIndexToEntity[ii->boundarySegmentIndex()] > 1)
                {
                  if (it != ipbsElems[i])
                  {
                    r_prime = ii->geometry().center();
                    dist = r-r_prime;
                    
                    // we are on a surface element and do integration for coulomb flux
                    // add surface charge contribution from all other surface elements but this one
                    // (using standard coulomb field formula)

                    switch ( sysParams.get_symmetry() )
                    {
                      case 2: // "spherical symmetry"
                      {
                        a = dist[0]*dist[0] + r[1]*r[1] + r_prime[1]*r_prime[1] + 2.0 * r[1] * r_prime[1];
                        b = 4.0 * r[1] * r_prime[1];
                        k = sqrt(b/a);
                        // if (a > 1e-3)
                        // {
                        k = sqrt(b/a);
                        E = gsl_sf_ellint_Ecomp (k, GSL_PREC_DOUBLE);
                        K = gsl_sf_ellint_Kcomp (k, GSL_PREC_DOUBLE);
                        // }
                        // else
                        // {
                        //   E = sysParams.pi / 2.0;
                        //   K = sysParams.pi / 2.0;
                        // }
                        E_field[0] = -2.0 * dist[0] / ((a-b)*sqrt(a)) * E;
                        E_field[1] = 2.0 * ( 2.0 * r_prime[1] * a - b * r_prime[1] - b * r[1])
                                                / ((a-b)*sqrt(a)*b) * E
                                + 2.0 * (-2.0 * r_prime[1] * a + 2.0 * b * r_prime[1]) /  ((a-b)*sqrt(a)*b) * K;
                        surfaceElem_flux = E_field * unitNormal;
                        surfaceElem_flux *= -2.0 * sysParams.get_bjerrum() * ii->geometry().volume() * r_prime[1] * boundary[boundaryIndexToEntity[ii->boundarySegmentIndex()]-2]->get_charge_density();
                        //std::cout << "surfaceElem_flux contributes " << surfaceElem_flux;
                      }
                      break;
                    }
                  }
                  else
                  {
                    itself_flux = 2. * sysParams.pi * sysParams.get_bjerrum() * boundary[boundaryIndexToEntity[ii->boundarySegmentIndex()]-2]->get_charge_density();
                    // std::cout << "ITSELFS FLUX: " << itself_flux;
                    summed_flux += itself_flux;
                  }
                  summed_flux += surfaceElem_flux;
              }
            }
          }
      }
      // std::cout << "Hello! surfaceElem_flux is now " << surfaceElem_flux;
    }
    // Include dielectrics! Multiply with dielectric factor...
    double diel_flux = dielectric_factor * summed_flux;
    fluxContainer[i] = summed_flux + diel_flux;
    // std::cout << std::endl <<" summed_flux is " << summed_flux << std::endl;
  }
}
