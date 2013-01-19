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


int LoadCReSISASCIIGrid(const char* filename, DataGrid** grid);
int LoadPNetCDFGrid(const char* filename, const char* varname, 
                    MPI_Offset start[], MPI_Offset count[],
                    DataGrid** grid);
MPI_Datatype NCTypeToMPI(nc_type type);

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
	char filename[256], varname[16][256];
  int nvars;
	int quit = 0;
	MPI_Offset slabstart[NC_MAX_DIMS], slabcount[NC_MAX_DIMS];
  const char* str, *str2;
  int pos = 0;
  int commsize, rank;

	MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	for(int i = 0; i < NC_MAX_DIMS; i++)
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
      printf("parsing vars '%s'...\n", optarg);
      if(strchr(optarg, ','))
      {
        nvars = 0;
        str = optarg;
        while(str && strlen(str) > 0)
        {
          str2 = strchr(str, ',');
          if(str2)
          {
            pos = str2 - str;
            strncpy(varname[nvars++], str, pos);
            printf("found var %s\n", varname[nvars-1]);
            str += pos + 1;
          }
          else
          {
            strcpy(varname[nvars++], str);
            printf("found var %s\n", varname[nvars-1]);
            str = 0;
          }
        }
      }
      else
      {
        strcpy(varname[0], optarg);
        nvars = 1;
      }
        
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

  printf("done with options\n");

  //if(nvars != commsize)
  //{
  //  fprintf(stderr, "nvars=%d != commsize=%d\n", nvars, commsize);
  //  exit(1);
  //}

  DataGrid* grid = 0;
  //LoadPNetCDFGrid(filename, varname[rank], slabstart, slabcount, &grid);
  LoadCReSISASCIIGrid(filename, &grid);
  if(grid)
  {
    GLWindow win("blah", 100, 100, 640, 480, false);
    win.AddLayer(new Layer(grid));
	
    while(win.IsActive())
      sleep(1);
  }
  else
  {
    fprintf(stderr, "Unable to load grid.\n");
  }

	MPI_Finalize();
	return 0;
}


#define READBUF_SIZE 4194304
#define WIDTH 720
#define HEIGHT 740
int LoadCReSISASCIIGrid(const char* filename, DataGrid** grid)
{
  int retval = NVN_NOERR;
  int ndims = 2;
  MPI_Offset dimlen[MAX_DIMS];
  char readbuf[READBUF_SIZE];
  float* buf = 0;
  int n;
  int line = 0;
  int x = 0;
  int y = 0;
  int readbufOffset = 0;
  int headerlines = 6;
  int mark = 0;
  Variant nodataValue;

  nodataValue.Type = VariantTypeFloat;
  nodataValue.Value.FloatVal = -9999.0f;

  dimlen[0] = HEIGHT;
  dimlen[1] = WIDTH;

  FILE* f = 0;
  f = fopen(filename, "r");
  if(0 == f)
  {
    retval = NVN_ERROR;
    fprintf(stderr, "Unable to open file '%s'.\n", filename);
  }

  buf = (float*)malloc(WIDTH * HEIGHT * sizeof(float));

  if(NVN_NOERR == retval)
  {
    while(! feof(f))
    {
      n = fread(readbuf + readbufOffset, sizeof(char), READBUF_SIZE - readbufOffset, f);

      mark = 0;
      for(int i = 0; i < n + readbufOffset; i++)
      {
        switch(readbuf[i])
        {
        case '\n':
          line ++;
          mark = i + 1;
          if(line > headerlines)
          {
            x = 0;
            y ++;
          }
          break;

        case ' ':
          if(line >= headerlines)
          {
            buf[y * WIDTH + x] = atof(readbuf + mark);
            mark = i + 1;
            x ++;
          }
          break;
        }
      }

      if(mark < n + readbufOffset)
      {
        memcpy(readbuf, readbuf + mark, n + readbufOffset - mark);
        readbufOffset = n + readbufOffset - mark;
      }
    }
  }

  if(NVN_NOERR == retval && grid)
  {
    *grid = new DataGrid(ndims, dimlen, MPI_FLOAT, buf);
    (*grid)->SetNodataValue(nodataValue);
  }

  return retval;
}


int LoadPNetCDFGrid(const char* filename, const char* varname, 
                    MPI_Offset start[], MPI_Offset count[],
                    DataGrid** grid)
{
  int retval = NVN_NOERR;
  int ncresult;
  int ncid, varid;
  nc_type vartype;
  int ndims, dimid[NC_MAX_DIMS];
  MPI_Offset varlen, dimlen[NC_MAX_DIMS];
  int gridndims;
  MPI_Offset griddimlen[NC_MAX_DIMS];
  MPI_Datatype gridtype;
  int typesize;
  void* buf = 0;

  if(NVN_NOERR == retval)
  {
    ncresult = ncmpi_open(MPI_COMM_WORLD, filename, 
                          NC_NOWRITE, MPI_INFO_NULL, &ncid);
    if(NC_NOERR != ncresult)
    {
      retval = ncresult;
      fprintf(stderr, "Failed to open '%s'.\n"
              "  Error message: %s\n",
              filename, ncmpi_strerror(ncresult));
    }
  }

  if(NVN_NOERR == retval)
  {
    ncresult = ncmpi_inq_varid(ncid, varname, &varid);
    if(NC_NOERR != ncresult)
    {
      retval = ncresult;
      fprintf(stderr, "Cannot find a variable named '%s' in '%s'.\n"
              "  Error message: %s\n",
              varname, filename, ncmpi_strerror(ncresult));
    }
  }

  if(NVN_NOERR == retval)
  {
    ncmpi_inq_var(ncid, varid, 0, &vartype, &ndims, dimid, 0);

    varlen = 1;
    gridndims = 0;
    for(int i = 0; i < ndims; i++)
    {
      ncmpi_inq_dimlen(ncid, dimid[i], &dimlen[i]);
      varlen *= dimlen[i];
      
      if(count[i] == -1)
        count[i] = dimlen[i] - start[i];
      
      if(count[i] > 1)
        griddimlen[gridndims++] = dimlen[i];
    }

    if(gridndims != 2)
    {
      fprintf(stderr, "Only two-dimensional data is supported, but the hyperslab "
              "defines a %d-dimensional space.\n", ndims);
      retval = NVN_ERROR;
    }
  }

  if(NVN_NOERR == retval)
  {
    gridtype = NCTypeToMPI(vartype);
    MPI_Type_size(gridtype, &typesize);
    buf = malloc(varlen * typesize);
    
    ncresult = ncmpi_get_vara_all(ncid, varid, start, count, 
                                  buf, varlen, gridtype);
    if(NC_NOERR != ncresult)
    {
      retval = ncresult;
      fprintf(stderr, "Failed to read data values.\n"
              "Error message: %s\n", ncmpi_strerror(ncresult));
    }
  }

  if(NVN_NOERR == retval && grid)
    *grid = new DataGrid(gridndims, griddimlen, gridtype, buf);

  return retval;
}


MPI_Datatype NCTypeToMPI(nc_type type)
{
	switch(type)
	{
	case NC_BYTE:
	case NC_CHAR:
		return MPI_CHAR;
	case NC_SHORT:
		return MPI_SHORT;
	case NC_INT:
		return MPI_INT;
	case NC_FLOAT:
		return MPI_FLOAT;
	case NC_DOUBLE:
		return MPI_DOUBLE;
	case NC_UBYTE:
		return MPI_BYTE;
	case NC_USHORT:
		return MPI_UNSIGNED_SHORT;
	case NC_UINT:
		return MPI_UNSIGNED;
	case NC_INT64:
		return MPI_LONG;
	case NC_UINT64:
		return MPI_UNSIGNED_LONG;
	default:
		fprintf(stderr, "NCTypeToMPI - Failed to map nc_type to MPI_Datatype\n");
		return MPI_BYTE;
	}
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
