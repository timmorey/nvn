/**
	 nvn.cpp - Created by Timothy Morey on 1/9/2013
 */

#include "gl-window.hpp"
#include "layer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <getopt.h>
#include <pnetcdf.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
	char c;
	int width = 800;
	int height = 600;
	char filename[256], varname[256];
	int quit = 0;
	nc_type vartype;
	int ncid, varid, ndims, dimid[NC_MAX_DIMS];
	MPI_Offset varlen, dimlen[NC_MAX_DIMS];
	int ncresult;
	int i;
	double* buf = 0;
	double min, max;
	int row, col;

	MPI_Init(&argc, &argv);

	while((c = getopt(argc, argv, "f:h:v:w:")) != -1)
	{
		switch(c)
		{
		case 'f':
			// Input data filename
			strcpy(filename, optarg);
			break;

		case 'h':
			// Display window height
			height = atoi(optarg);
			break;

		case 'v':
			// Variable name
			strcpy(varname, optarg);
			break;

		case 'w':
			// Display window width
			width = atoi(optarg);
			break;

		default:
			fprintf(stderr, "Unrecognized option: %c\n", optarg);
			break;
		}
	}

	ncresult = ncmpi_open(MPI_COMM_WORLD, filename, NC_NOWRITE, MPI_INFO_NULL, &ncid);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Failed to open '%s'.\n"
						"  Error message: %s\n",
						filename, ncmpi_strerror(ncresult));
		exit(1);
	}

	ncresult = ncmpi_inq_varid(ncid, varname, &varid);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Cannot find a variable named '%s' in '%s'.\n"
						"  Error message: %s\n",
						varname, filename, ncmpi_strerror(ncresult));
		exit(1);
	}

	ncresult = ncmpi_inq_var(ncid, varid, 0, &vartype, &ndims, dimid, 0);
	if(ndims != 2)
	{
		fprintf(stderr, "Variable '%s' has %d dimensions.  "
						"Only two-dimensional variables are supported at this time.\n",
						varname, ndims);
		exit(1);
	}
	else if(NC_DOUBLE != vartype)
	{
		fprintf(stderr, "Invalid variable type.  Only double is currently supported.");
		exit(1);
	}

	varlen = 1;
	for(i = 0; i < ndims; i++)
	{
		ncresult = ncmpi_inq_dimlen(ncid, dimid[i], &dimlen[i]);
		varlen *= dimlen[i];
	}

	buf = (double*)malloc(varlen * sizeof(double));
	ncresult = ncmpi_get_var_double_all(ncid, varid, buf);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Failed to read data values.\n"
						"Error message: %s\n", ncmpi_strerror(ncresult));
		exit(1);
	}

	GLWindow win("blah", 100, 100, 640, 480, false);
	win.AddLayer(new Layer(dimlen[1], dimlen[0], MPI_DOUBLE, buf));
	
	while(win.IsActive())
		sleep(1);

	MPI_Finalize();
	return 0;
}
