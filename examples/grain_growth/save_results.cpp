#include "save_results.h"
#include <stdlib.h>
#include <cstdio>
#include <png.h>
#include <map>
#include <iostream>
using namespace std;

struct rgb_t
{
	unsigned char r, g, b;
};

void save_results(int* grains, int w, int h, const char* file)
{
	srand(0);
	int k = 0;
	map<int,rgb_t> colors;
	FILE* fp = fopen(file,"wb");
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png,fp);
	png_set_IHDR(
		png,
		info,
		w,h,
		8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);
	png_set_filler(png,0,PNG_FILLER_AFTER);
	png_bytep *row_pointers = new png_byte*[h];
	for (int i = 0; i < h; i++)
	{
		row_pointers[i] = new png_byte[4*w];
		for (int j = 0; j < w; j++)
		{
			rgb_t color;
			auto iter = colors.find(grains[k]);
			if (iter != colors.end())
				color = (*(iter)).second;
			else
			{
				color.r = rand()%256;
				color.g = rand()%256;
				color.b = rand()%256;
				colors[grains[k]] = color;
			}
			row_pointers[i][4*j] = color.r;
			row_pointers[i][4*j+1] = color.g; 
			row_pointers[i][4*j+2] = color.b;
			k++;
		}
	}
	png_write_image(png,row_pointers);
	png_write_end(png,NULL);
	for (int i = 0; i < h; i++)
		delete [] row_pointers[i];
	delete [] row_pointers;
	fclose(fp);
	png_destroy_write_struct(&png,&info);
}

