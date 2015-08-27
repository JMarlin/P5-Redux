#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]){
        
        FILE *kernelImage, *driverImage;
        unsigned int i, j, kernelSize, driverCount, *driverSize; 
        int tempChar;

        if(argc < 3){
                printf("Usage: %s kernel.bin driver1 [driver2 driver3 ...]\n", argv[0]);
                return 0;
        }

        /*
        steps: 
                1) Load the kernel image and exit if the dvrpkg offset is nonzero
                2) Read through the kernel until we hit EOF, counting every byte on the way
                   (while doing this, write all of those bytes to the output file)
                3) Write the drivers count byte to the end of the output image 
                4) Write pad-zeros for each required imgsize entry
                5) Open the next driver image, append it to the output image while counting its size
                6) Save the size of the driver image
                7) Close the image and repeat from 5 if there are more drivers in the list
                8) Close and reopen the out kernel, fsetpos to the start of the imgsize entries
                   and write each of the driver sizes in order into the file
        */

        kernelImage = fopen(argv[1], "rb+");
        if(!kernelImage){
                printf("Error: Couldn't open kernel image '%s'!\n", argv[1]);
                return -1;
        }                 

        kernelSize = 0;
        while((tempChar = fgetc(kernelImage)) != EOF){
                kernelSize++;        
                if(kernelSize > 5 && kernelSize < 10){
                        if(tempChar){
                                printf("Error: Kernel image '%s' is already packaged!\n", argv[1]);
                                fclose(kernelImage);
                                return -1;
                        }
                }
        }

        fseek(kernelImage, 5, SEEK_SET);
        fputc((kernelSize & 0xFF), kernelImage);
        fputc(((kernelSize >> 8) & 0xFF), kernelImage);
        fputc(((kernelSize >> 16) & 0xFF), kernelImage);
        fputc(((kernelSize >> 24) & 0xFF), kernelImage);

        fseek(kernelImage, 0, SEEK_END);
        driverCount = argc - 2;
        fputc((driverCount & 0xFF), kernelImage);

        for(i = 0; i < driverCount; i++){
                for(j = 0; j < 4; j++){
                        fputc(0, kernelImage);
                }
        }

        driverSize = malloc(4*driverCount);
        for(i = 2; i < argc; i++){
                driverSize[i-2] = 0;
                driverImage = fopen(argv[i], "rb");
                if(!driverImage){
                        printf("Error: Could not open driver image '%s'!\n", argv[i]);
                        free(driverSize);
                        return -1;
                }
        
                while((tempChar = fgetc(driverImage)) != EOF){
                        driverSize[i-2]++;
                        fputc(tempChar, kernelImage);
                }

                fclose(driverImage);
        }
        
        fseek(kernelImage, kernelSize + 1, SEEK_SET);
        for(i = 0; i < driverCount; i++){
                fputc((driverSize[i] & 0xFF), kernelImage);
                fputc(((driverSize[i] >> 8) & 0xFF), kernelImage);
                fputc(((driverSize[i] >> 16) & 0xFF), kernelImage);
                fputc(((driverSize[i] >> 24) & 0xFF), kernelImage);
        }

        free(driverSize);
        fclose(kernelImage);

}
