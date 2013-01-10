/**
	 nvn.c - Created by Timothy Morey on 1/9/2013
 */

#include <float.h>
#include <getopt.h>
#include <SDL/SDL.h>
#include <mpi.h>
#include <pnetcdf.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	char c;
	int width = 800;
	int height = 600;
	char filename[256], varname[256];
	SDL_Surface* window = 0;
	SDL_Event event;
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
	SDL_Init(SDL_INIT_EVERYTHING);

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
		goto shutdown;
	}

	ncresult = ncmpi_inq_varid(ncid, varname, &varid);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Cannot find a variable named '%s' in '%s'.\n"
						"  Error message: %s\n",
						varname, filename, ncmpi_strerror(ncresult));
		goto shutdown;
	}

	ncresult = ncmpi_inq_var(ncid, varid, 0, &vartype, &ndims, dimid, 0);
	if(ndims != 2)
	{
		fprintf(stderr, "Variable '%s' has %d dimensions.  "
						"Only two-dimensional variables are supported at this time.\n",
						varname, ndims);
		goto shutdown;
	}
	else if(NC_DOUBLE != vartype)
	{
		fprintf(stderr, "Invalid variable type.  Only double is currently supported.");
		goto shutdown;
	}

	varlen = 1;
	for(i = 0; i < ndims; i++)
	{
		ncresult = ncmpi_inq_dimlen(ncid, dimid[i], &dimlen[i]);
		varlen *= dimlen[i];
	}

	buf = malloc(varlen * sizeof(double));
	ncresult = ncmpi_get_var_double_all(ncid, varid, buf);
	if(NC_NOERR != ncresult)
	{
		fprintf(stderr, "Failed to read data values.\n"
						"Error message: %s\n", ncmpi_strerror(ncresult));
		goto shutdown;
	}

	window = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);

	min = DBL_MAX;
	max = DBL_MIN;
	for(i = 0; i < varlen; i++)
	{
		if(buf[i] < min) min = buf[i];
		if(buf[i] > max) max = buf[i];
	}

	Uint32* pixels = (Uint32*)window->pixels;
	for(i = 0; i < varlen; i++)
	{
		row = i / dimlen[1];
		col = i % dimlen[1];
		pixels[row * window->w + col] = 0x00010000 * (int)((buf[i] - min) / (max - min) * 255);
		pixels[row * window->w + col] += 0x000000ff - (int)((buf[i] - min) / (max - min) * 255);
	}

	SDL_Flip(window);

	while(!quit)
	{
		if(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				printf("Quit event received\n");
				quit = 1;
				break;
			}
		}
	}

shutdown:
	SDL_Quit();
	MPI_Finalize();
	return 0;
}
