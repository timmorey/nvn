/**
  color-ramp.h - Created by Timothy Morey on 5/27/2012
*/

#ifndef __COLOR_RAMP_H
#define __COLOR_RAMP_H

#include <libxml/tree.h>

#include "variant.h"

#define MAX_COLOR_STOPS 32

#define RED 0
#define GREEN 1
#define BLUE 2
#define ALPHA 3

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
  ColorRampTypeSmooth,  // Creates a smooth gradient of colors by blending the
                        // stop colors when necessary.

  ColorRampTypeStepped  
} ColorRampType;

typedef enum
{
  ColorStopValueTypeAbsolute,
  ColorStopValueTypePercentage
} ColorStopValueType;

typedef struct
{
  Variant Value;
  int Color;
} ColorStop;

typedef struct
{
  ColorRampType RampType;
  ColorStopValueType StopValueType;
  ColorStop Stops[MAX_COLOR_STOPS];
  int NStops;
} ColorRamp;

extern const ColorRamp DefaultColorRamp;

int GetColor(ColorRamp ramp, Variant value, Variant minval, Variant maxval);

char GetR(int color);
char GetG(int color);
char GetB(int color);
char GetA(int color);

int InsertColorStop(ColorRamp* ramp, ColorStop stop);

int ParseColorRamp(xmlNodePtr root, ColorRamp* ramp);

int ParseColorStop(xmlNodePtr root, ColorStop* stop);

int SaveColorRamp(ColorRamp ramp, xmlNodePtr* root);

int SaveColorStop(ColorStop stop, xmlNodePtr* root);

#ifdef __cplusplus
}
#endif

#endif
