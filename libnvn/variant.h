/**
  variant.h - Created by Timothy Morey on 6/1/2012
*/

#ifndef __VARIANT_H
#define __VARIANT_H


#include "nvn.h"

#include <libxml/tree.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
  The VariantType enum gives the supported variant types, which are aligned
  with the NetCDF nc_type enum.
*/
typedef enum
{
  VariantTypeNull   = 0,

  VariantTypeByte   = 1,
  VariantTypeChar   = 2,
  VariantTypeShort  = 3,
  VariantTypeInt    = 4,
  VariantTypeFloat  = 5,
  VariantTypeDouble = 6
} VariantType;

typedef struct
{
  VariantType Type;

  union
  {
    unsigned char ByteVal;
    char CharVal;
    short ShortVal;
    int IntVal;
    float FloatVal;
    double DoubleVal;
  } Value;

} Variant;

VariantType MPITypeToVariantType(MPI_Datatype type);

int ParseVariant(xmlNodePtr root, Variant* var);

Variant MaxVariant(VariantType vartype);

Variant MinVariant(VariantType vartype);

int SaveVariant(Variant var, xmlNodePtr* root);

int VariantCompare(Variant v1, Variant v2);

int VariantInRange(Variant v, Variant min, Variant max);

int VariantIsNearlyEqual(Variant v1, Variant v2);

MPI_Datatype VariantTypeToMPIType(VariantType type);

double VariantValueAsDouble(Variant v);

float VariantValueAsFloat(Variant v);

#ifdef __cplusplus
}
#endif

#endif
