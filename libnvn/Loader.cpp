/**
   Loader.cpp - Created by Timothy Morey on 2/5/2013
 */


#include "nvn.h"

#include "DataGrid.hpp"
#include "Loader.hpp"

#include <pnetcdf.h>
#include <string.h>


MPI_Datatype NCTypeToMPI(nc_type type);


int DetermineFileFormat(const char* filename, FileFormat* format)
{
  int retval = NVN_NOERR;

  if(format)
  {
    *format = FileFormatUnknown;
    
    const char* ext = strrchr(filename, '.');
    if(ext)
    {
      if(0 == strcmp(ext, ".nc"))
      {
        // Then it may be CDF1, CDF2, CDF5, or HDF5
        int ncid, ncresult, ncformat;
        
        ncresult = ncmpi_open(MPI_COMM_WORLD, filename, 
                              NC_NOWRITE, MPI_INFO_NULL, &ncid);
        if(NC_NOERR == ncresult)
        {
          ncresult = ncmpi_inq_format(ncid, &ncformat);
          if(NC_NOERR == ncresult)
          {
            switch(ncformat)
            {
            case NC_FORMAT_CLASSIC:
              *format = FileFormatCDF1;
              break;
            case NC_FORMAT_64BIT:
              *format = FileFormatCDF2;
              break;
            case NC_FORMAT_64BIT_DATA:
              *format = FileFormatCDF5;
              break;
            }
          }

          ncmpi_close(ncid);
        }
        else
        {
          // If we add support for Unidata's NetCDF, then we should try to open
          // the file with it here to see if the file is HDF5.
        }    
      }
      else if(0 == strcmp(ext, ".txt"))
      {
        // Then it may be a CReSIS grid - just assume it is until we have need
        // to be more precise.
        *format = FileFormatCReSISGrid;
      }
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

#define READBUF_SIZE 4194304

int LoadCReSISASCIIGrid(const char* filename, DataGrid** grid)
{
  int retval = NVN_NOERR;
  int ndims = 2;
  MPI_Offset dimlen[MAX_DIMS];
  char dimnames[MAX_DIMS][MAX_NAME];
  char readbuf[READBUF_SIZE];
  float* buf = 0;
  int n;
  int x = 0;
  int y = 0;
  int readbufOffset = 0;
  int mark = 0;
  Variant nodataValue;
  int width, height;
  float xllcorner, yllcorner, cellsize;

  nodataValue.Type = VariantTypeFloat;
  nodataValue.Value.FloatVal = -666.0f;

  FILE* f = 0;
  f = fopen(filename, "r");
  if(0 == f)
  {
    retval = NVN_ERROR;
    fprintf(stderr, "Unable to open file '%s'.\n", filename);
  }

  // Read the header
  if(NVN_NOERR == retval)
  {
    if(! feof(f)) fscanf(f, "ncols %d\n", &width);
    if(! feof(f)) fscanf(f, "nrows %d\n", &height);
    if(! feof(f)) fscanf(f, "xllcorner %f\n", &xllcorner);
    if(! feof(f)) fscanf(f, "yllcorner %f\n", &yllcorner);
    if(! feof(f)) fscanf(f, "cellsize %f\n", &cellsize);
    if(! feof(f)) fscanf(f, "NODATA_value %f\n", &nodataValue.Value.FloatVal);
  }

  strcpy(dimnames[0], "y");
  dimlen[0] = height;

  strcpy(dimnames[1], "x");
  dimlen[1] = width;

  buf = (float*)malloc(width * height * sizeof(float));

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
          mark = i + 1;
          x = 0;
          y ++;
          break;

        case ' ':
          buf[y * width + x] = atof(readbuf + mark);
          mark = i + 1;
          x ++;
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
    *grid = new DataGrid(ndims, dimnames, dimlen, MPI_FLOAT, buf);
    (*grid)->SetNodataValue(nodataValue);
  }

  if(f)
  {
    fclose(f);
    f = 0;
  }

  return retval;
}


int LoadPNetCDFGrid(const char* filename, const char* varname, 
                    MPI_Offset start[], MPI_Offset count[], MPI_Offset stride[],
                    DataGrid** grid)
{
  int retval = NVN_NOERR;
  int ncresult;
  int ncid = 0, varid = 0;
  nc_type vartype;
  int ndims, dimid[NC_MAX_DIMS];
  char dimname[MAX_DIMS][MAX_NAME];
  MPI_Offset varlen, dimlen[NC_MAX_DIMS];
  int gridndims;
  MPI_Offset griddimlen[NC_MAX_DIMS];
  char griddimname[MAX_DIMS][MAX_NAME];
  MPI_Datatype gridtype;
  int typesize;
  void* buf = 0;

  nc_type nodataType;
  Variant nodataValue;
  bool hasNodataValue = false;

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
      ncmpi_inq_dimname(ncid, dimid[i], dimname[i]);
      ncmpi_inq_dimlen(ncid, dimid[i], &dimlen[i]);
      
      if(count[i] < 1 ||
         count[i] * stride[i] > dimlen[i] - start[i])
      {
        count[i] = (dimlen[i] - start[i]) / stride[i];
        if((dimlen[i] - start[i]) % stride[i] > 0) count[i]++;
      }
      
      if(count[i] > 1)
      {
        varlen *= count[i];
        griddimlen[gridndims] = count[i];
        strcpy(griddimname[gridndims], dimname[i]);
        gridndims ++;
      }
    }

    ncresult = ncmpi_inq_atttype(ncid, varid, "_FillValue", &nodataType);
    if(NC_NOERR == ncresult)
    {
      if(nodataType == NC_FLOAT)
      {
        hasNodataValue = true;
        nodataValue.Type = VariantTypeFloat;
        ncmpi_get_att_float(ncid, varid, "_FillValue", &nodataValue.Value.FloatVal);
      }
      else if(nodataType == NC_DOUBLE)
      {
        hasNodataValue = true;
        nodataValue.Type = VariantTypeDouble;
        ncmpi_get_att_double(ncid, varid, "_FillValue", &nodataValue.Value.DoubleVal);
      }
      else
      {
        fprintf(stderr, "Unsupported NoData type: %d.\n", nodataType);
      }
    }
  }

  if(NVN_NOERR == retval)
  {
    gridtype = NCTypeToMPI(vartype);
    MPI_Type_size(gridtype, &typesize);
    buf = malloc(varlen * typesize);
    
    ncresult = ncmpi_get_vars_all(ncid, varid, start, count, stride,
                                  buf, varlen, gridtype);
    if(NC_NOERR != ncresult)
    {
      retval = ncresult;
      fprintf(stderr, "Failed to read data values.\n"
              "Error message: %s\n", ncmpi_strerror(ncresult));
    }
  }

  if(NVN_NOERR == retval && grid)
  {
    *grid = new DataGrid(gridndims, griddimname, griddimlen, gridtype, buf);
    if(hasNodataValue)
      (*grid)->SetNodataValue(nodataValue);
  }

  if(ncid)
  {
    ncmpi_close(ncid);
    ncid = 0;
  }

  return retval;
}


/******************************************************************************
 * Local function definitions
 ******************************************************************************/

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
