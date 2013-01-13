/**
  color-ramp.c - Created by Timothy Morey on 5/27/2012
*/

#include <libxml/tree.h>
#include <string.h>

#include "color-ramp.h"
#include "variant.h"

#ifdef __WINDOWS__
#pragma warning(disable:4996)
#endif

const ColorRamp DefaultColorRamp =
{
	ColorRampTypeSmooth,
	ColorStopValueTypePercentage,
	{ // Stops
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 0.0f }
			},
			0xFFFFFFFF
		},
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 0.2f }
			},
			0xFFFF0000
		},
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 0.4f }
			},
			0xFFFFFF00
		},
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 0.6f }
			},
			0xFF00FF00
		},
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 0.8f }
			},
			0xFF00FFFF
		},
		{ // ColorStop
			{ // Value
				VariantTypeFloat,
				{ .FloatVal = 1.0f }
			},
			0xFF0000FF
		}
	},
	6
};


int GetColor(ColorRamp ramp, Variant value, Variant minval, Variant maxval)
{
  int retval = 0;

  if(ramp.NStops > 0)
  {
    int understop = 0;
    int overstop = 0;
    double v = VariantValueAsDouble(value);
    double min = VariantValueAsDouble(minval);
    double max = VariantValueAsDouble(maxval);

    if(ColorStopValueTypePercentage == ramp.StopValueType)
      if(min == max)
        v = 0.0;  // Avoid divide by 0
      else
        v = (v - min) / (max - min);

    while(VariantValueAsDouble(ramp.Stops[overstop].Value) < v && overstop < ramp.NStops - 1)
      overstop++;

    understop = overstop;

    while(VariantValueAsDouble(ramp.Stops[understop].Value) > v && understop > 0)
      understop--;

    if(understop == overstop ||
      ColorRampTypeStepped == ramp.RampType)
    {
      retval = ramp.Stops[understop].Color;
    }
    else
    {
      unsigned char* undercolor = (unsigned char*)&ramp.Stops[understop].Color;
      unsigned char* overcolor = (unsigned char*)&ramp.Stops[overstop].Color;
      double underval = VariantValueAsDouble(ramp.Stops[understop].Value);
      double overval = VariantValueAsDouble(ramp.Stops[overstop].Value);
      double x = 0.0;
      if(overval != underval)  // Avoid divide by 0
        x = (v - underval) / (overval - underval);

      ((unsigned char*)&retval)[RED] = undercolor[RED] + 
        (int)(x * ((double)overcolor[RED] - (double)undercolor[RED]));
      ((unsigned char*)&retval)[GREEN] = undercolor[GREEN] + 
        (int)(x * ((double)overcolor[GREEN] - (double)undercolor[GREEN]));
      ((unsigned char*)&retval)[BLUE] = undercolor[BLUE] + 
        (int)(x * ((double)overcolor[BLUE] - (double)undercolor[BLUE]));
      ((unsigned char*)&retval)[ALPHA] = undercolor[ALPHA] + 
        (int)(x * ((double)overcolor[ALPHA] - (double)undercolor[ALPHA]));
    }
  }

  return retval;
}

int InsertColorStop(ColorRamp* ramp, ColorStop stop)
{
  int retval = 0;
  int i = 0;

  if(ramp && ramp->NStops < MAX_COLOR_STOPS)
  {
    ramp->Stops[ramp->NStops] = stop;
    ramp->NStops++;

    for(i = ramp->NStops - 1; i > 0; i--)
    {
      if(0 > VariantCompare(ramp->Stops[i].Value, ramp->Stops[i-1].Value))
      {
        stop = ramp->Stops[i-1];
        ramp->Stops[i-1] = ramp->Stops[i];
        ramp->Stops[i] = stop;
      }
      else
      {
        break;
      }
    }

    retval = ramp->NStops;
  }

  return retval;
}

char GetR(int color)
{
	return ((char*)&color)[RED];
}

char GetG(int color)
{
	return ((char*)&color)[GREEN];
}

char GetB(int color)
{
	return ((char*)&color)[BLUE];
}

char GetA(int color)
{
	return ((char*)&color)[ALPHA];
}

int ParseColorRamp(xmlNodePtr root, ColorRamp* ramp)
{
  int retval = 0;
  xmlChar* value;
  xmlNodePtr child = 0;

  if(root && ramp)
  {
    memset(ramp, 0, sizeof(ColorRamp));

    value = xmlGetProp(root, BAD_CAST "RampType");
    ramp->RampType = (ColorRampType)atoi((char*)value);
    xmlFree(value);
    
    value = xmlGetProp(root, BAD_CAST "StopValueType");
    ramp->StopValueType = (ColorStopValueType)atoi((char*)value);
    xmlFree(value);

    child = root->children;
    while(child)
    {
      if(0 == xmlStrcmp(BAD_CAST "ColorStop", child->name))
      {
        ColorStop stop;
        ParseColorStop(child, &stop);
        InsertColorStop(ramp, stop);
      }

      child = child->next;
    }
  }

  return retval;
}

int ParseColorStop(xmlNodePtr root, ColorStop* stop)
{
  int retval = 0;
  xmlChar* value;
  xmlNodePtr child;

  if(root && stop)
  {
    memset(stop, 0, sizeof(ColorStop));

    value = xmlGetProp(root, BAD_CAST "Color");
    stop->Color = atoi((char*)value);
    xmlFree(value);

    child = root->children;
    while(child)
    {
      if(0 == xmlStrcmp(BAD_CAST "Value", child->name))
      {
        ParseVariant(child, &stop->Value);
      }

      child = child->next;
    }
  }

  return retval;
}

int SaveColorRamp(ColorRamp ramp, xmlNodePtr* root)
{
  int retval = 0;
  int i = 0;
  char temp[32];
  xmlNodePtr child = 0;

  if(root)
  {
    if(! *root)
      *root = xmlNewNode(0, BAD_CAST "ColorRamp");

    sprintf(temp, "%d", ramp.RampType);
    xmlNewProp(*root, BAD_CAST "RampType", BAD_CAST temp);

    sprintf(temp, "%d", ramp.StopValueType);
    xmlNewProp(*root, BAD_CAST "StopValueType", BAD_CAST temp);

    for(i = 0; i < ramp.NStops; i++)
    {
      child = 0;
      SaveColorStop(ramp.Stops[i], &child);
      xmlAddChild(*root, child);
    }

    retval = 1;
  }

  return retval;
}

int SaveColorStop(ColorStop stop, xmlNodePtr* root)
{
  int retval = 0;
  char temp[32];
  xmlNodePtr child = 0;

  if(root)
  {
    if(! *root)
      *root = xmlNewNode(0, BAD_CAST "ColorStop");

    sprintf(temp, "%d", stop.Color);
    xmlNewProp(*root, "Color", BAD_CAST temp);

    child = xmlNewNode(0, BAD_CAST "Value");
    SaveVariant(stop.Value, &child);
    xmlAddChild(*root, child);

    retval = 1;
  }

  return retval;
}
