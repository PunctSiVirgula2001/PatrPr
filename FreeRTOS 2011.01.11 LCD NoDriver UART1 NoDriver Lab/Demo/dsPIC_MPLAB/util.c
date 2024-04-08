#include "util.h"

//map function that maps the in range to the out range
// it also has clamping in case the x value is out of bounds 
//of the in interval
float map_with_clamp(float x, float in_min, float in_max, float out_min, float out_max){
	if(x > in_max){
		x = in_max;
	}else if(x < in_min){
		x = in_min;
	}
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}