/**
	 nvn.cpp - Created by Timothy Morey on 1/9/2013
 */

#include "gl-window.hpp"
#include "layer.hpp"
#include "nvn.h"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <getopt.h>
#include <pnetcdf.h>
#include <stdio.h>
#include <stdlib.h>


/**
	 Parses a string representation of a hyperslab, which is a comma-delimited
	 sequence of integers.

	 @param str A null terminated string that describes a hyperslab.
	 @param slab An array large enough to hold the parsed hyperslab definition 
	 (NC_MAX_DIMS should be long enough).

	 @return 0 if the operation was successful, and an error code otherwise.
 */
int ParseHyperslab(const char* str, MPI_Offset slab[]);

int main(int argc, char* argv[])
{
	char c;
	int width = 800;
	int height = 600;
	char filename[256], varname[256];
	int quit = 0;
	nc_type vartype;
	int ncid, varid, ndims, dimid[NC_MAX_DIMS];
	int slabdims;
	MPI_Offset varlen, dimlen[NC_MAX_DIMS];
	int ncresult;
	int i;
	double* buf = 0;
	double min, max;
	int row, col;
	MPI_Offset slabstart[NC_MAX_DIMS], slabcount[NC_MAX_DIMS];

	MPI_Init(&argc, &argv);

	for(i = 0; i < NC_MAX_DIMS; i++)
	{
		slabstart[i] = 0;
		slabcount[i] = -1;
	}

	while((c = getopt(argc, argv, "c:f:h:s:v:w:")) != -1)
	{
		switch(c)
		{
		case 'c':
			// Hyperslab count to define the data we're rendering
			ParseHyperslab(optarg, slabcount);
			break;

		case 'f':
			// Input data filename
			strcpy(filename, optarg);
			break;

		case 'h':
			// Display window height
			height = atoi(optarg);
			break;
			
		case 's':
			// Hyperslab start to define the data we're rendering
			ParseHyperslab(optarg, slabstart);
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
	if(NC_DOUBLE != vartype)
	{
		fprintf(stderr, "Invalid variable type.  Only double is currently supported.");
		exit(1);
	}

	varlen = 1;
	slabdims = 0;
	width = -1;
	height = -1;
	for(i = 0; i < ndims; i++)
	{
		ncresult = ncmpi_inq_dimlen(ncid, dimid[i], &dimlen[i]);
		varlen *= dimlen[i];

		if(slabcount[i] == -1)
			slabcount[i] = dimlen[i] - slabstart[i];

		if(slabcount[i] > 1)
		{
			slabdims++;
			if(height < 0) height = slabcount[i];
			else width = slabcount[i];
		}
	}

	if(slabdims != 2)
	{
		fprintf(stderr, "Only two-dimensional data is supported, but the hyperslab "
						"defines a %d-dimensional space.\n", ndims);
		exit(1);
	}

	buf = (double*)malloc(varlen * sizeof(double));
	ncresult = ncmpi_get_vara_double_all(ncid, varid, slabstart, slabcount, buf);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Failed to read data values.\n"
						"Error message: %s\n", ncmpi_strerror(ncresult));
		exit(1);
	}

	GLWindow win("blah", 100, 100, 640, 480, false);
	win.AddLayer(new Layer(width, height, MPI_DOUBLE, buf));
	
	while(win.IsActive())
		sleep(1);

	MPI_Finalize();
	return 0;
}

int ParseHyperslab(const char* str, MPI_Offset slab[])
{
	int retval = NVN_NOERR;
	int i;
	int d = 0;

	while(str && strlen(str) > 0)
	{
		slab[d++] = atoi(str);
		str = strchr(str, ',');
		if(str)
			str++;
	}

	return retval;
}