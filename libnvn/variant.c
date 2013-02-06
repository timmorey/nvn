/**
   variant.c - Created by Timothy Morey on 6/1/2012
*/

#define MPICH_SKIP_MPICXX
#include <mpi.h>

#include <float.h>
#include <libxml/tree.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "variant.h"

#ifdef __WINDOWS__
#pragma warning(disable:4996)
#endif

#define EPSILON 1e-6

int ParseVariant(xmlNodePtr root, Variant* var)
{
  int retval = 0;
  xmlChar* value;
  
  if(root && var)
  {
    memset(var, 0, sizeof(Variant));

    value = xmlGetProp(root, BAD_CAST "Type");
    var->Type = atoi((char*)value);
    xmlFree(value);
    
    value = xmlGetProp(root, BAD_CAST "Value");
    switch(var->Type)
    {
    case VariantTypeByte:
      var->Value.ByteVal = (unsigned char)atoi((char*)value); break;
    case VariantTypeChar:
      var->Value.CharVal = (char)atoi((char*)value); break;
    case VariantTypeShort:
      var->Value.ShortVal = (short)atoi((char*)value); break;
    case VariantTypeDouble:
      var->Value.DoubleVal = atof((char*)value); break;
    case VariantTypeFloat:
      var->Value.FloatVal = (float)atof((char*)value); break;
    case VariantTypeInt:
      var->Value.IntVal = atoi((char*)value); break;
    default:
      fprintf(stderr, "ParseVariant - unsupported variant type: %d", var->Type);
      break;
    }
    xmlFree(value);
  }
  
  return retval;
}

Variant MaxVariant(MPI_Datatype vartype)
{
  Variant retval = { 0, 0 };
  
  switch(vartype)
  {
  case MPI_BYTE:
    retval.Type = VariantTypeByte;
    retval.Value.ByteVal = UCHAR_MAX;
    break;
    
  case MPI_CHAR:
    retval.Type = VariantTypeChar;
    retval.Value.CharVal = CHAR_MAX;
    break;
    
  case MPI_SHORT:
    retval.Type = VariantTypeShort;
    retval.Value.ShortVal = SHRT_MAX;
    break;
    
  case MPI_DOUBLE:
    retval.Type = VariantTypeDouble;
    retval.Value.DoubleVal = DBL_MAX;
    break;
    
  case MPI_FLOAT:
    retval.Type = VariantTypeFloat;
    retval.Value.FloatVal = FLT_MAX;
    break;
    
  case MPI_INT:
    retval.Type = VariantTypeInt;
    retval.Value.IntVal = INT_MAX;
    break;
  }
  
  return retval;
}

Variant MinVariant(MPI_Datatype vartype)
{
  Variant retval = { 0, 0 };
  
  switch(vartype)
  {
  case MPI_BYTE:
    retval.Type = VariantTypeByte;
    retval.Value.ByteVal = 0;
    break;
    
  case MPI_CHAR:
    retval.Type = VariantTypeChar;
    retval.Value.CharVal = CHAR_MIN;
    break;
    
  case MPI_SHORT:
    retval.Type = VariantTypeShort;
    retval.Value.ShortVal = SHRT_MIN;
    break;
    
  case MPI_DOUBLE:
    retval.Type = VariantTypeDouble;
    retval.Value.DoubleVal = DBL_MIN;
    break;
    
  case MPI_FLOAT:
    retval.Type = VariantTypeFloat;
    retval.Value.FloatVal = FLT_MIN;
    break;
    
  case MPI_INT:
    retval.Type = VariantTypeInt;
    retval.Value.IntVal = INT_MIN;
    break;
  }
  
  return retval;
}

int SaveVariant(Variant var, xmlNodePtr* root)
{
  int retval = 0;
  char temp[32];

  if(root)
  {
    if(! *root)
      *root = xmlNewNode(0, BAD_CAST "Variant");

    sprintf(temp, "%d", var.Type);
    xmlNewProp(*root, BAD_CAST "Type", BAD_CAST temp);
    
    switch(var.Type)
    {
    case VariantTypeByte:
      sprintf(temp, "%u", var.Value.ByteVal); break;
    case VariantTypeChar:
      sprintf(temp, "%d", var.Value.CharVal); break;
    case VariantTypeShort:
      sprintf(temp, "%hd", var.Value.ShortVal); break;
    case VariantTypeDouble:
      sprintf(temp, "%f", var.Value.DoubleVal); break;
		case VariantTypeFloat:
      sprintf(temp, "%f", var.Value.FloatVal); break;
		case VariantTypeInt:
      sprintf(temp, "%d", var.Value.IntVal); break;
    default:
      sprintf(temp, "%d", "unsupported type");
      fprintf(stderr, "SaveVariant - unsupported variant type: %d", var.Type);
      break;
    }
    xmlNewProp(*root, BAD_CAST "Value", BAD_CAST temp);
    
    retval = 1;
  }
  
  return retval;
}

int VariantCompare(Variant v1, Variant v2)
{
  int retval = 0;

  if(v1.Type == v2.Type)
  {
    switch(v1.Type)
    {
		case VariantTypeByte:
      if(v1.Value.ByteVal < v2.Value.ByteVal)
        retval = -1;
      else if(v1.Value.ByteVal > v2.Value.ByteVal)
        retval = 1;
      break;
      
		case VariantTypeChar:
      if(v1.Value.CharVal < v2.Value.CharVal)
        retval = -1;
      else if(v1.Value.CharVal > v2.Value.CharVal)
        retval = 1;
      break;
      
		case VariantTypeShort:
      if(v1.Value.ShortVal < v2.Value.ShortVal)
        retval = -1;
      else if(v1.Value.ShortVal > v2.Value.ShortVal)
        retval = 1;
      break;
      
    case VariantTypeDouble:
      if(v1.Value.DoubleVal < v2.Value.DoubleVal)
        retval = -1;
      else if(v1.Value.DoubleVal > v2.Value.DoubleVal)
        retval = 1;
      break;
      
		case VariantTypeFloat:
      if(v1.Value.FloatVal < v2.Value.FloatVal)
        retval = -1;
      else if(v1.Value.FloatVal > v2.Value.FloatVal)
        retval = 1;
      break;
      
    case VariantTypeInt:
      if(v1.Value.IntVal < v2.Value.IntVal)
        retval = -1;
      else if(v1.Value.IntVal > v2.Value.IntVal)
        retval = 1;
      break;
    }
  }
  
  return retval;
}

int VariantInRange(Variant v, Variant min, Variant max)
{
  int retval = 0;
  
  if(v.Type == min.Type && min.Type == max.Type)
  {
    retval = 0 <= VariantCompare(max, v) &&
      0 <= VariantCompare(v, min);
  }
  
  return retval;
}

int VariantIsNearlyEqual(Variant v1, Variant v2)
{
  int retval = 0;
  
  if(v1.Type == v2.Type)
  {
    switch(v1.Type)
    {
    case VariantTypeByte:
      if(v1.Value.ByteVal == v2.Value.ByteVal)
        retval = 1;
      break;
      
    case VariantTypeChar:
      if(v1.Value.CharVal == v2.Value.CharVal)
        retval = 1;
      break;
      
    case VariantTypeShort:
      if(v1.Value.ShortVal == v2.Value.ShortVal)
        retval = 1;
      break;
      
    case VariantTypeDouble:
      if(fabs(v1.Value.DoubleVal - v2.Value.DoubleVal) < EPSILON)
        retval = 1;
      break;
      
    case VariantTypeFloat:
      if(fabsf(v1.Value.FloatVal - v2.Value.FloatVal) < EPSILON)
        retval = 1;
      break;

    case VariantTypeInt:
      if(v1.Value.IntVal == v2.Value.IntVal)
        retval = 1;
      break;
    }
  }
  
  return retval;
}

double VariantValueAsDouble(Variant v)
{
  double retval = 0.0;
  
  switch(v.Type)
  {
  case VariantTypeByte:
    retval = (double)v.Value.ByteVal; break;
  case VariantTypeChar:
    retval = (double)v.Value.CharVal; break;
  case VariantTypeShort:
    retval = (double)v.Value.ShortVal; break;
  case VariantTypeDouble:
    retval = v.Value.DoubleVal; break;
  case VariantTypeFloat:
    retval = (double)v.Value.FloatVal; break;
  case VariantTypeInt:
    retval = (double)v.Value.IntVal; break;
  }

  return retval;
}

float VariantValueAsFloat(Variant v)
{
  float retval = 0.0;
  
  switch(v.Type)
  {
  case VariantTypeByte:
    retval = (float)v.Value.ByteVal; break;
  case VariantTypeChar:
    retval = (float)v.Value.CharVal; break;
  case VariantTypeShort:
    retval = (float)v.Value.ShortVal; break;
  case VariantTypeDouble:
    retval = (float)v.Value.DoubleVal; break;
  case VariantTypeFloat:
    retval = v.Value.FloatVal; break;
  case VariantTypeInt:
    retval = (float)v.Value.IntVal; break;
  }

  return retval;
}
