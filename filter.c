/* Program: filter.c
 * Author: Saif Fadhel
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pixel { 
    unsigned char r, g, b; 
} PPMPixel;

typedef struct image {
  PPMPixel **data;       
  unsigned int width, height;
  unsigned int max_color;     
} PPMImage;

PPMImage *readPPM(const char *file_name) {

    char magicNum[3]; /* Every ppm file has a magic number of two characters defining it */
    PPMImage *ppm = NULL; /* .ppm image to extract values from */
    int pixelsRead;

    /* Opens file for reading */
    FILE *fp = fopen(file_name, "r");
    if (fp==NULL) {
        fprintf(stderr, "Unable to open file \"%s\"\n", file_name);
        return NULL;
    } else {
        fscanf(fp,"%s\n",magicNum);
    }

    /* If we have a P3 .ppm image, store width, height and max color */
    if(strcmp(magicNum, "P3") == 0){
        ppm = malloc(sizeof(PPMImage));
        fscanf(fp,"%d", &ppm->width);
        fscanf(fp,"%d", &ppm->height);
        fscanf(fp,"%d", &ppm->max_color);
    }

    /* Allocates space for each pixel */
    ppm->data = malloc(sizeof(PPMPixel*)*ppm->height);
    int i;
    for (i=0; i < ppm->height; i++)
        ppm->data[i] = malloc(sizeof(PPMPixel)*ppm->width);

    /* Reads each pixel */
    int j;
    for (i=0; i < ppm->height; i++) {
        for (j=0; j < ppm->width; j++) {
            pixelsRead = fscanf(fp, "%hhu %hhu %hhu", &(ppm->data[i][j].r), &(ppm->data[i][j].g), &(ppm->data[i][j].b));
        }
    }

   /* Close file pointer and return image */
   fclose(fp); 
   return ppm; 
}

/* writes a .ppm image to an output file */
void write_ppm(const char*file_name, const PPMImage *ppm){
   
    /* Opens file for writing to record magic number, width, height, and max color while encoding the image in ASCII */
    FILE *fp = fopen(file_name, "w");
    if (fp==NULL) {
        fprintf(stderr, "Unable to open file \"%s\"\n", file_name);
        return;
    } else {
        fprintf(fp, "P3\n" );
        fprintf(fp, "%u %u\n", ppm->width, ppm->height);
        fprintf(fp, "%u\n", ppm->max_color);

        int i, j;
        for (i = 0; i < ppm->height; i++){
            for (j = 0; j < ppm->width; j++){
                fprintf(fp, "%u %u %u ", ppm->data[i][j].r, ppm->data[i][j].g, ppm->data[i][j].b);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
}



int main(int argc, char **argv) {

    if (argc!=4) {
        printf("Usage ./filter input.ppm kernel output.ppm");
        return -1;
    }

    /* Stores all files to be referenced */
    const char *input_file = argv[1];
    const char *kernel_file = argv[2];
    const char *output_file = argv[3];

    /* Obtains all information about given input image to be manipulated in main */
    PPMImage* image = readPPM(input_file);

    /* Identifying kernel file variables */
    int n=0, i, j, loop_limit;
    float scale;
    float filterArray[n*n];
    float filterArray2D[n][n];

    /* Opening kernel file */
    FILE *fp = fopen(kernel_file, "r"); 
    if (fp==NULL) {
        fprintf(stderr, "Unable to open file \"%s\"\n", kernel_file);
        return -2;
    }

    /* Extracing information from kernel file */
    fscanf(fp, "%d", &n);
    loop_limit=n;
    fscanf(fp, "%f", &scale);
    for (j=0; j<n*n; j++) {
        fscanf(fp, "%f", &filterArray[j]);
    }

    /* Converts 1D matrix A to 2D matrix A2 for easier indexing */
    int y, t;
    for (y=0;y<loop_limit;y++){
        for(t=0;t<loop_limit;t++){
            filterArray2D[t][y]=filterArray[t*loop_limit+y];
        }
    }

    float pixel_sum_r=0, pixel_sum_green=0, pixel_sum_b=0;

    int x=0, indexImageX, indexImageY, indexFiltX, indexFiltY; 

    /* Computing the convolution */
    for(x = 0; x < image->width; x++) {
        for(y = 0; y < image->height; y++) {

            double pixel_sum_red = 0.0, pixel_sum_green = 0.0, pixel_sum_blue = 0.0;

            for(indexFiltY = 0; indexFiltY < loop_limit; indexFiltY++) {
                for(indexFiltX = 0; indexFiltX < loop_limit; indexFiltX++)
                {

                indexImageX = (x - loop_limit / 2 + indexFiltX + image->width) % image->width;
                indexImageY = (y - loop_limit / 2 + indexFiltY + image->height) % image->height;
                
                pixel_sum_red += image->data[indexImageY][indexImageX].r * filterArray2D[indexFiltY][indexFiltX];
                pixel_sum_green += image->data[indexImageY][indexImageX].g * filterArray2D[indexFiltY][indexFiltX];
                pixel_sum_blue += image->data[indexImageY][indexImageX].b * filterArray2D[indexFiltY][indexFiltX];
                
                }
            }

            int temp_red = 1/scale * pixel_sum_red;
            int temp_green = 1/scale * pixel_sum_green;
            int temp_blue = 1/scale * pixel_sum_blue;

            image->data[y][x].r = abs(temp_red) < 255 ? abs(temp_red) : 255;
            image->data[y][x].g = abs(temp_red) < 255 ? abs(temp_green) : 255;
            image->data[y][x].b = abs(temp_red) < 255 ? abs(temp_blue) : 255;
        }
    }

    /* Close file */
    fclose(fp);

    /* After kernel has been convoluted with image, output to a new .ppm file */
    write_ppm(output_file, image); 

    return 0;
}
