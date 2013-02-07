/**
	 nvn.cpp - Created by Timothy Morey on 1/9/2013
 */


#include "nvn.h"

#include <getopt.h>
#include <mpi.h>
#include <pnetcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
  MPI_Offset slabstart[MAX_DIMS], slabcount[MAX_DIMS];
  const char* str, *str2;
  int pos = 0;
  int commsize, rank;
  int i;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(i = 0; i < MAX_DIMS; i++)
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

  NVN_Window vis;
  NVN_Model model;
  NVN_DataGrid grid;
  NVN_Layer layer;
  NVN_DataGridDescriptor desc;
  NVN_Err nvnresult;

  strcpy(desc.Filename, filename);
  strcpy(desc.Varname, varname[rank]);
  memcpy(desc.Start, slabstart, MAX_DIMS * sizeof(MPI_Offset));
  memcpy(desc.Count, slabcount, MAX_DIMS * sizeof(MPI_Offset));

  nvnresult = NVN_LoadDataGrid(desc, &grid);

  if(NVN_NOERR == nvnresult)
  {
    nvnresult = NVN_CreateShadedSurfaceGridLayer(grid, &layer);
    nvnresult = NVN_CreateModel(&model);
    nvnresult = NVN_AddLayer(model, layer);
    nvnresult = NVN_CreateWindow("nvn", 100, 100, 640, 480, 0, &vis);
    nvnresult = NVN_ShowModel(vis, model);

    while(NVN_IsWindowActiveP(vis))
      sleep(1);
  }
  else
  {
    fprintf(stderr, "Unable to load grid.\n");
  }

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
