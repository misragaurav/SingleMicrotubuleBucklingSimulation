#include "proto.h"

#define animation
//#define animation_small

int main (int argc, char **argv)
{
  struct params *params;
  struct centro *centro;
  struct mt *mt;
  struct seg *ps;
  struct rod *rod;
  struct rodparams *rparams;

  FILE  *file_ptr=0;
  int seed, n, k;
  int totalsegs;
  real dt;

  if (argc < 7)
    {printf("\n Too few arguments to the C code, bailing out. \n"); exit(0); }

  Nmt  	 = atoi(argv[1]);
  dt 	 = atof(argv[2]);
  ds     = atof(argv[3]);
  iter 	 = atoi(argv[4]);	  //number of iterations
  iprint = atoi(argv[5]);	  //how many iterations to save after
  seed	 = atoi(argv[6]);

  strcpy(datadir,"data");

  strcat(datadir,"-");
  strcat(datadir,argv[1]);
  strcat(datadir,"-");
  strcat(datadir,argv[2]);
  strcat(datadir,"-");
  strcat(datadir,argv[3]);
  strcat(datadir,"-");
  strcat(datadir,argv[4]);
  strcat(datadir,"-");
  strcat(datadir,argv[5]);
  strcat(datadir,"-");
  strcat(datadir,argv[6]);

  char command[64]={"mkdir "};
  strcat(command,datadir);
  if (system(command) != 0) // You must check if the command was successful
    {printf("mkdir not successful, bailing out \n");exit(1);}

  // GSL random number generator setup
  gsl_rng_env_setup();
  rng = gsl_rng_alloc(gsl_rng_default);
  // printf ("generator type: %s \nseed = %lu \n", gsl_rng_name(rng), gsl_rng_default_seed);

  //
  Nmax = 40; // The maximum number of microtubules that can be spawned

  // Memory allocation
  params  = (struct params    *) calloc(1,   sizeof(struct params   ));
  centro  = (struct centro    *) calloc(1,   sizeof(struct centro   ));
  mt      = (struct mt        *) calloc(Nmt, sizeof(struct mt       ));
  rod     = (struct rod       *) calloc(Nmt, sizeof(struct rod      ));
  rparams = (struct rodparams *) calloc(1,   sizeof(struct rodparams));

  initmt(params, centro, mt);
  initrod(params, rparams, rod, dt);

  for (n=0; n<Nmt; n++)
    {
      tempinterfacein(&mt[n], &rod[n], centro, rparams);
      tempinterfaceout(&mt[n], &rod[n]);
    }

  for (t=0; t<iter; t++)
    {
  //printing part

#ifdef animation
      char animationxyz[64];
      strcpy(animationxyz, datadir);
      strcat(animationxyz, "/a.xyz");
      file_ptr  = fopen(animationxyz, "a");

#ifdef animation_small
      if (t%(10*iprint) == 0)
#else
	if (t%(iprint) == 0)
#endif
	  {
	    totalsegs = 0;
	    for(n=0; n<Nmt; n++)
	      totalsegs = totalsegs + mt[n].N;

	    fprintf (file_ptr,"%3i \nAtom_name \n", totalsegs);

	    totalsegs = 0;

	    for(n=0; n<Nmt; n++)
	      {
		for (k = 0; k < mt[n].N; k++)
		  {
		    ps = &(mt[n].seg[k]);
		    totalsegs = totalsegs + 1;
		    fprintf (file_ptr, "AtomNum%i \t %.16e \t %.16e \t %.16e \n", totalsegs, ps->rx, ps->ry, ps->rz);
		  }
	      }
	  }
      fclose (file_ptr);
#endif

      if (t%(iprint) == 0)
	{
	  char centrofile[64];
	  strcpy(centrofile, datadir);
	  strcat(centrofile, "/c.xyz");
	  file_ptr = fopen(centrofile, "a");
	  fprintf (file_ptr,"%i \nAtom_name \n", 1);
	  fprintf (file_ptr,"AtomNum%i %.16e \t %.16e \t %.16e \n", 1, centro->rx, centro->ry, centro->rz);
	  fclose (file_ptr);
	}

      for (n=0; n<Nmt; n++)
	{
	  //	  polymerize(&mt[n], params, dt);
	  rodflex(rparams, &rod[n], t);
	  tempinterfaceout(&mt[n], &rod[n]);
	}

      movecentro(params, centro, mt, dt);
    }

  // Free memory

  gsl_rng_free(rng);

  for (n=0; n<Nmt; n++)
    {
      free(mt[n].seg);
      free(rod[n].elems);
      free(rod[n].nodes);
    }

  free(mt);
  free(rod);
  free(params);
  free(rparams);
  free(centro);

  return 0;
}
