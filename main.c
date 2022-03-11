#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void byte_flip(uint8_t *horizontal, uint8_t *vertical)
{
    memset(vertical, 0x00, 8);
    for(uint8_t i=0; i<8; i++)
    {
        for(uint8_t b=0; b<8; b++)
        {
            vertical[i] |= (horizontal[b] & (1<<i)) ? (1<<b) : 0;
        }
    }
}

void collect_horizontal(uint8_t *horizontal, uint8_t *data, uint16_t rowsize,uint16_t yPtr,uint16_t xPtr, uint16_t start,uint8_t color1)
{
    for(uint8_t i=0; i<8; i++)
    {
        if(color1)
        {
            horizontal[i]= data[-i * rowsize - yPtr * 8 * rowsize + xPtr + start];//if black is 1 do nothing
        }
        else
        {
            horizontal[i]= ~data[-i * rowsize - yPtr * 8 * rowsize + xPtr + start];//if white is 1 we need to flip the bits
        }
    }
}

void collect_hor(uint8_t *horizontal,uint8_t *data,uint16_t rowsize,uint16_t yPtr,uint16_t start,uint8_t color1)
{
    for(uint8_t i=0; i<rowsize; i++)
    {
        if(color1)
        {
            horizontal[i]= data[-yPtr * rowsize + i + start];//if black is 1 do nothing
        }
        else
        {
            horizontal[i]= ~data[-yPtr * rowsize + i + start];//if white is 1 we need to flip the bits
        }
    }

}

void print_data(uint8_t *vertPix, FILE *printFile, uint16_t rowWidth, uint32_t height, uint32_t width,uint16_t printPerRow, uint8_t compact, uint8_t packetsPerRow)
{
    static uint8_t count2 = 0;
    uint8_t end = 0;
    static uint16_t count=0;
    static uint16_t PrintedTotal = 0;//total numbers printed
    static uint16_t PrintedRow = 0;//numbers printed on the current row
    //fprintf(printFile,"\n\t");
    if(compact){
        printPerRow = width*packetsPerRow+1;
    }
    for(uint8_t i=0; i<8; i++)
    {
        if(count<width) //if not padding
        {
            if (vertPix[7-i]==0)
            {
                fprintf(printFile, "   0");
            }
            else
            {
                fprintf(printFile,"0x%02x",vertPix[7-i]);
            }

            //printf(printFile,"0x%.2x",vertPix[7-i]);
            if(!(++PrintedTotal % width))//if end of a width is reached print curly brackets
            {
                if(PrintedTotal/(height/8)==width)
                {
                    if(compact){
                        fprintf(printFile,"}\n}");
                    }else{
                        fprintf(printFile,"\n\t}\n}");
                    }
                    end = 1;
                }
                else
                {
                    if(compact)
                    {
                        if(count2<(packetsPerRow-1))
                        {
                            fprintf(printFile,"},{ ");
                            count2++;
                        }
                        else
                        {
                            fprintf(printFile,"},\n\t{ ");
                            count2=0;
                        }
                    
                    }
                    else
                    {
                        fprintf(printFile,"\n\t},\n\t{\n\t ");
                        PrintedRow=-1;
                    }
                }
            }
            else
            {
                fprintf(printFile,", ");
                //printf(printFile,", ");
            }
            PrintedRow++;
            if(!(PrintedRow % printPerRow)&&PrintedRow&&(!end)&&(!compact))
            {
                fprintf(printFile,"\n\t ");    //deals with printPerRow here
                //printf(printFile,"\n\t ");    //deals with printPerRow here
                PrintedRow=0;
            }
        }
        count++;
        if(count==rowWidth)
        {
            count=0;
        }
    }
}

void print_data_hor(uint8_t *horBytes, FILE *printFile, uint16_t rowWidth, uint32_t height, uint32_t width,uint16_t printPerRow, uint8_t compact, uint8_t packetsPerRow)
{
    static uint8_t count2 = 0;
    uint8_t end = 0;
    static uint16_t count=0;
    static uint16_t PrintedTotal = 0;//total numbers printed
    static uint16_t PrintedRow = 0;//numbers printed on the current row
    int numToPrint = width/8;
    if (compact)
    {
        printPerRow = rowWidth*packetsPerRow+1;
    }
    uint8_t mask = numToPrint*8-width;
    mask = 256-pow(2,mask);
    for(uint8_t i=0; i<numToPrint; i++)
    {
        if (horBytes[i]==0)
        {
            fprintf(printFile, "0x00");
        }
        else
        {
            uint8_t temp = horBytes[i];
            if(i == numToPrint-1)
            {
                temp = temp & mask;
            }
            if(temp == 0)
            {
                fprintf(printFile, "   0");
            }
            else
            {
                fprintf(printFile,"0x%02x",temp);
            }
        }
        PrintedTotal += 1;
        if(i == numToPrint-1)//if end of a width is reached print curly brackets
        {
            if((PrintedTotal)/(height)==numToPrint)
            {
                if(compact){
                    fprintf(printFile,"}\n}");
                }else{
                    fprintf(printFile,"\n\t}\n}");
                }
                end = 1;
            }
            else
            {
                if(compact)
                {
                    if(count2<(packetsPerRow-1))
                    {
                        fprintf(printFile,"},{ ");
                        count2++;
                    }
                    else
                    {
                        fprintf(printFile,"},\n\t{ ");
                        count2=0;
                    }
                }
                else
                {
                    fprintf(printFile,"\n\t},\n\t{\n\t ");
                    PrintedRow=-1;
                }
            }
        }
        else
        {
            fprintf(printFile,", ");
            //printf(printFile,", ");
        }
        PrintedRow++;
        if(!(PrintedRow % printPerRow)&&PrintedRow&&(!end)&&(!compact))
        {
            fprintf(printFile,"\n\t ");    //deals with printPerRow here
            PrintedRow=0;
        }
        count++;
        if(count==rowWidth)
        {
            count=0;
        }
    }
}

int main(int argc, char *argv[])
{

    uint8_t addHeight = 0;
    uint16_t addBytes = 0;
    uint8_t vertPix[8];
    uint8_t horPix[8];
    uint32_t Header[15];
    uint16_t fileType[1];
    uint8_t compact = 0;//set to 1 to print compact mode
    uint8_t packetsPerRow = 1;//for compact mode will print this many packets per row
    uint16_t printPerRow = 16;//set how many numbers you want per row does not work with compact mode
    #define ORIENT_H    0
    #define ORIENT_V    1
    int orient = ORIENT_H;
    char *inFileName;
    char *ppr;//print per row string
    char *kpr;//packets per row string
    if(argc<2)
    { 
        printf("Error: No filename specified");
        exit(0);
    }
    if(*argv[1] == '-')
    {
        //////////////////////////////////////////////////arg 1
        if(*(argv[1]+1)== 'v')
        {
            orient = ORIENT_V;
        }else if(*(argv[1]+1)== 'c')
        {
            compact = 1;
        }else{
            printf("Error: invalid argument only '-c' and '-v' are acceptable");
        }
        if(argc < 3)
        {
            printf("Error: No filename specified");
            exit(0);
        }
        ////////////////////////////////////////////////////arg 2
        if(*argv[2] == '-'){
            //v or c here 
            if(*(argv[2]+1)== 'v')
            {
                orient = ORIENT_V;
            }else if(*(argv[2]+1)== 'c')
            {
                compact = 1;
            }else{
                printf("Error: invalid argument only '-c' and '-v' are acceptable");
            }
            if(argc<4){
                printf("Error: No filename specified");
                exit(0);
            }
            /////////////////////////////////////////////////////////////////arg 3 (or 3 and 4)
            if(argc<5){
                inFileName = argv[3];
            }else{
                kpr = argv[3];
                packetsPerRow = atoi(kpr);
                inFileName = argv[4];
            }
            
        }else{
            //either filename is here or kpr or ppr
            if (argc<4){
                inFileName = argv[2];
            }else{
                if(compact){
                    //c was first arg
                    //kpr
                    kpr = argv[2];
                    packetsPerRow = atoi(kpr);
                    inFileName = argv[3];
                }else{
                    //v was first arg
                    //ppr
                    ppr = argv[2];
                    printPerRow = atoi(ppr);
                    inFileName = argv[3];
                }
            }
        }
    }
    else
    {
        if(argc<3){
            inFileName = argv[1];
        }else{
            ppr = argv[1];
            printPerRow = atoi(ppr);
            inFileName = argv[2];
        }
    }
    char fileName[40];
    strcpy(fileName,inFileName);
    char fileName2[40]; //extra string to store the filename without .BMP on the end
    memset(fileName2,0x00,40);
    FILE*bmpIn=fopen(fileName,"r");
    uint8_t len = strlen(fileName);
    strxfrm(fileName2, fileName, len-4);//cutting off .bmp so it can be used to name the printed array
    strcat(fileName2,"_bmp");
    fread(fileType, 2, 1, bmpIn);
    if(fileType[0]!=0x4D42)
    {
        printf("Error: file version is not BM, or invalid cmd format");
        return 0;
    }
    fread(Header, 4, 15, bmpIn);
    //uint32_t fileSize = *Header;//file size in bytes
    uint32_t offset = Header[2];//offset for start of bitmap data
    uint32_t headerSize = Header[3];//check to see if this is 40
    if(headerSize!=40)
    {
        printf("Error header size not 40 bytes");
        return 0;
    }
    uint32_t width = Header[4];
    uint32_t height = Header[5];
    uint32_t bitsPP = Header[6] & 0xFFFF;//bits per pixel
    if(bitsPP>1)
    {
        printf("Error: Bits per pixel not 1");
        return 0;
    }
    uint32_t sizeBM = Header[8];
    //uint32_t numberOfColors = Header[11];
    uint32_t color1 = Header[13];
    FILE*printFile = fopen("output.c", "w");
    if((color1&0xFFFFFFFFFFFF)==0)
    {
        color1=0;//white is color 1
    }
    else
    {
        color1=1;//black is color 1
    }
    fseek(bmpIn, offset, SEEK_SET);//move to offset for bitmap data
    uint8_t *data;
    uint8_t rowsize = width / 32;
    if(width % 32) rowsize++;
    rowsize *= 4;//rowsize is the number of bytes per row horizontally
    if(height%8)
    {
        //printf("height is not divisible by 8 adding vertical 0 padding...\n");
        addHeight = 8-height%8;
        addBytes = addHeight*rowsize;
    }
    data = (uint8_t *) malloc(sizeBM+addBytes);//allocate size of bitmap bytes plus extra padding bytes
    if(color1)
    {
        memset(data, 0x00, sizeBM+addBytes);//zero when white is zero
    }
    else
    {
        memset(data, 0xff, sizeBM+addBytes);//ff when white is one
    }
    fread(data+addBytes, 1, sizeBM, bmpIn);//read in all bytes, added the offset addBytes so that the padding is at the bottom of the output file, BMP files are read from the bottom so the top right corner of the bitmap is the end of the array
    fclose(bmpIn);
    uint16_t newHeight = height + addHeight;//new height including added padding
    if(orient)
    {
        fprintf(printFile,"const uint8_t _%s[%u][%u]={\n\t{ ",fileName2, newHeight/8, width);//array header
    }
    else
    {
        fprintf(printFile,"const uint8_t _%s[%u][%u]={\n\t{ ",fileName2, height, rowsize);//array header
    }
    if(!compact){
        fprintf(printFile,"\n\t ");
    }
    if(orient)
    {
        for(uint16_t yPtr=0; yPtr < newHeight / 8; yPtr++)
        {
            for(uint16_t xPtr=0; xPtr < rowsize; xPtr++)
            {
                collect_horizontal(horPix, data, rowsize, yPtr, xPtr, sizeBM+addBytes-rowsize, color1);//collect horizontal function for byteflip
                byte_flip(horPix, vertPix);
                print_data(vertPix, printFile, rowsize*8, newHeight, width,printPerRow, compact, packetsPerRow);//prints 8 bytes that were just converted if they are not padding
            }
        }
    }
    else
    {
        uint8_t *horBytes;
        horBytes = (uint8_t *) malloc(rowsize);
        for(uint16_t yPtr=0; yPtr < height; yPtr++)
        {
            collect_hor(horBytes,data,rowsize,yPtr,sizeBM+addBytes-rowsize,color1);//collect horizontal to directly print it
            print_data_hor(horBytes, printFile, rowsize, height, width,printPerRow, compact, packetsPerRow);//prints 8 bytes that were just converted if they are not padding
        }
    }
    fprintf(printFile, ";");
    fclose(printFile);
    system("output.c");
    free(data);
    return 0;
}
