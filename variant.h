/**
  variant.h - Created by Timothy Morey on 6/1/2012
*/

#ifndef __VARIANT_H
#define __VARIANT_H

#include <libxml/tree.h>

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
  int Type;

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

int ParseVariant(xmlNodePtr root, Variant* var);

Variant MaxVariant(int vartype);

Variant MinVariant(int vartype);

int SaveVariant(Variant var, xmlNodePtr* root);

int VariantCompare(Variant v1, Variant v2);

int VariantInRange(Variant v, Variant min, Variant max);

int VariantIsNearlyEqual(Variant v1, Variant v2);

double VariantValueAsDouble(Variant v);

#ifdef __cplusplus
}
#endif

#endif
