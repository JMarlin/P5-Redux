#include <memory.h>
#include <stdio.h>

typedef struct fileMeta {
    unsigned int length;
    unsigned char nameLength;
    char* name;
} fileMeta;


void fputDword(unsigned int value, FILE* file) {

    fputc((value & 0xFF), file);
    fputc(((value >> 8) & 0xFF), file);
    fputc(((value >> 16) & 0xFF), file);
    fputc(((value >> 24) & 0xFF), file);
}


int main(int argc, char* argv[]){
        
        FILE *diskImage, *fileImage;
        unsigned int i, j, fileCount, offset; 
        int tempChar;
        fileMeta* file;
        
        if(argc < 3){
                printf("Usage: %s out.rd file1 [file2 file3 ...]\n", argv[0]);
                return 0;
        }

        /*
        steps: 
                1) Malloc the array of fileMetas 
                2) For each file listed
                    - Open the file
                    - Scan to find the file size, store in file->length
                    - Set file->name to the argval
                    - Scan to find the length of the argval, store in file->nameLength
                    - Close the file
                3) offset = 4 + (fileCount*9) //Length of count int plus each file descriptor less the size of the filename strings
                4) For i = 0 to fileCount
                    - offset += file[i].nameLength
                5) Open disk file
                6) Write the file count
                7) For each file entry:
                    - write offset 
                    - offset += file->length
                    - write file->length
                    - write file->nameLength
                    - write file->name
                8) For each file entry:
                    - open the file
                    - write the content of the file 
                    - close the file
                9) Close the disk file
        */
          
        fileCount = argc - 2;
        file = (fileMeta*)malloc(4*fileCount);
        
        printf("%d files\n", fileCount);
        
        for(i = 0; i < fileCount; i++){

                fileImage = fopen(argv[i+2], "rb");
                if(!fileImage){
                        printf("Error: Could not open file '%s'!\n", argv[i+2]);
                        free(file);
                        return -1;
                }
        
                file[i].length = 0;
                while((tempChar = fgetc(fileImage)) != EOF)
                        file[i].length++;

                file[i].name = argv[i];        
                        
                for(file[i].nameLength = 0; file[i].name[file[i].nameLength]; file[i].nameLength++);
                
                fclose(fileImage);
        }
        
        offset = 4 + (fileCount * 9);
        
        for(i = 0; i < fileCount; i++) 
            offset += file[i].nameLength;
        
        
        diskImage = fopen(argv[1], "wb");
        if(!diskImage){
                printf("Error: Couldn't open disk image '%s'!\n", argv[1]);
                free(file);
                return -1;
        }   
               
        fputDword(fileCount, diskImage);
 
        for(i = 0; i < fileCount; i++) {
            
            printf("-------------\n");
            printf("offset 0x%x\n", offset);
            printf("length 0x%x\n", file[i].length);
            printf("strlen 0x%x\n", file[i].nameLength);
            printf("name %s\n", file[i].name);
            
            fputDword(offset, diskImage);
            fputDword(file[i].length, diskImage);
            fputc(file[i].nameLength, diskImage);
            
            for(j = 0; j < file[i].nameLength; j++)
                fputc(file[i].name[j], diskImage);
                
            offset += file[i].length;
        }
 
        for(i = 0; i < fileCount; i++) {
        
            fileImage = fopen(argv[i+2], "rb");
            if(!fileImage){
                printf("Error: Could not open file '%s'!\n", argv[i+2]);
                free(file);
                fclose(diskImage);
                return -1;
            }
            
            for(j = 0; j < file[i].length; j++) 
                fputc(fgetc(fileImage), diskImage);
                
            fclose(fileImage);
        }
 
        free(file);
        fclose(diskImage);
}
