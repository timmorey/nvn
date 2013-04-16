/**
   Loader.hpp - Created by Timothy Morey on 2/5/2013
 */

#ifndef __LOADER_HPP__
#define __LOADER_HPP__

enum FileFormat
{
  FileFormatUnknown = 0,

  FileFormatCDF1,
  FileFormatCDF2,
  FileFormatCDF5,
  FileFormatHDF5,
  FileFormatCReSISGrid
};

class DataGrid;

int DetermineFileFormat(const char* filename, FileFormat* format);

int LoadCReSISASCIIGrid(const char* filename, DataGrid** grid);

int LoadPNetCDFGrid(const char* filename, const char* varname, 
                    MPI_Offset start[], MPI_Offset count[], MPI_Offset stride[],
                    DataGrid** grid);

#endif
