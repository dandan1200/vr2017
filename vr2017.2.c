/* Name: Daniel Chorev  
 * unikey: dcho3009
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <ctype.h>


// Function signatures
int check_delims(char d1[], char d2[], char d3[]);
int calc_parity_byte(int delims[4]);
int * check_valid_packet_and_perform_swizzle(int curr_packet[4], int current_packet_index, int swizzle, int last_valid[3]);

int main(int argc, char* argv[]){
    //Checking the command line args:
        
        // Check correct number of command line arguments
        if (argc != 5) {
            printf("Error. Incorrect number of command line arguments.\n");
            return 0;
        }

        // Check file exists.
        FILE *file;
        if ((file = fopen(argv[1], "r")) == 0){
            printf("Error. File does not exist.\n");
            fclose(file);
            return 0;
        }
        fclose(file);

        // Check delimiter bytes
        if (check_delims(argv[2],argv[3],argv[4]) == 0){
            printf("Error. Invalid delimiters.\n");
            return 0;

        }

        //Convert delimiters to ints
        int delims_int[3] = {
            (int)strtol(argv[2], NULL, 0),
            (int)strtol(argv[3], NULL, 0),
            (int)strtol(argv[4], NULL, 0)
        };

    //************************
    //OPEN FILE
    //************************

    file = fopen(argv[1],"rb");

    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    rewind(file);

    //Read contents of file into array for easier access.
    // int file_contents[1000];
    // for (int i = 0; i < size; i++){
    //     file_contents[i] = fgetc(file);
        
    // }

    // //Add delim to end of file if not already present
    // if (
    //     file_contents[size-4] != delims_int[0] ||
    //     file_contents[size-3] != delims_int[1] ||
    //     file_contents[size-2] != delims_int[2] ||
    //     file_contents[size-1] != calc_parity_byte((int[4]){file_contents[size-4],file_contents[size-3],file_contents[size-2],0})
    // )
    // {
    //     file_contents[size] = delims_int[0];
    //     file_contents[size+1] = delims_int[1];
    //     file_contents[size+2] = delims_int[2];
    //     file_contents[size+3] = calc_parity_byte((int[4]){delims_int[0],delims_int[1],delims_int[2],0});
    //     size+=3;
    // }

    //Initiate variables for chunk handling
    int delim_counter = 0;
    int current_delim = 0;
    double x_chunk_avg = 0;
    double y_chunk_avg = 0;
    double z_chunk_avg = 0;
    int prev_valid_packet[3];
    int curr_delim_check[4];
    int curr_packet[4];

    //Traverse all file contents in groups of 3 bytes.
    for (int i = 0; i < size-1; i++){
        //Read in 4 bytes at a time.
        curr_delim_check[0] = fgetc(file);
        curr_delim_check[1] = fgetc(file);
        curr_delim_check[2] = fgetc(file);
        curr_delim_check[3] = fgetc(file);
        fseek(file,-3L,SEEK_CUR);
        
        //printf("%d %d %d %d\n",curr_delim_check[0],curr_delim_check[1],curr_delim_check[2],curr_delim_check[3]);
        //Check current 3 byte group for delimiter plus checksum.

        if (
            (curr_delim_check[0] == delims_int[0] &&
            curr_delim_check[1]== delims_int[1] &&
            curr_delim_check[2] == delims_int[2] &&
            curr_delim_check[3] == calc_parity_byte((int[4]){curr_delim_check[0],curr_delim_check[1],curr_delim_check[2],0})) || curr_delim_check[3] == -1
            
        )
        {

            //BEGIN ANALYSING CHUNK:
            int chunk = current_delim;

            


            if (curr_delim_check[3] == -1) {
                chunk = current_delim;
                i = size;
            }
            //printf("%d %d\n", i, chunk);

            printf("Chunk: %d at offset: %d\n", delim_counter,current_delim);

            if (((i-chunk) % 5) != 0) {

                printf("Error! chunk not multiple of 5 bytes.\n");

            } else {
                //Reset chunk average
                x_chunk_avg = 0;
                y_chunk_avg = 0;
                z_chunk_avg = 0;

                int packet = 0;
                int invalid_packets = 0;
                int current_packet_index;

                //Repeat logic for each packet within chunk.
                for (packet = 0; packet < (i-chunk)/5; packet++){
                    
                    current_packet_index = (chunk + packet*5);
                    printf("    Packet: %d\n", packet);
                    fseek(file,current_packet_index,SEEK_SET);
                    //Checksum for packet
                    if (
                        calc_parity_byte((int[4]){fgetc(file),
                                                fgetc(file),
                                                fgetc(file),
                                                fgetc(file)}
                                            ) == fgetc(file)
                        ) 
                        {
                        fseek(file,current_packet_index,SEEK_SET);
                        curr_packet[0] = fgetc(file);
                        curr_packet[1] = fgetc(file);
                        curr_packet[2] = fgetc(file);
                        curr_packet[3] = fgetc(file);
                        printf("        Data before swizzle -> B0: %d, B1: %d, B2: %d\n", curr_packet[0],curr_packet[1],curr_packet[0]);
                        //Set initial last valid packet with sentinal values    
                        if (packet == 0) {
                            prev_valid_packet[0] = -1000;
                            prev_valid_packet[1] = -1000;
                            prev_valid_packet[2] = -1000;
                        }

                        //Calculate swizzle
                        int * res = check_valid_packet_and_perform_swizzle(curr_packet,current_packet_index,curr_packet[3],prev_valid_packet);
                        printf("        Data after swizzle -> X: %d, Y: %d, Z: %d\n",*res,*(res+1),*(res+2));
                        
                        //Calculate average/whether to ignore packet.
                        if (*(res + 3) == 3) {
                            prev_valid_packet[0] = *res;
                            prev_valid_packet[1] = *(res+1);
                            prev_valid_packet[2] = *(res+2);

                            x_chunk_avg += *res;
                            y_chunk_avg += *(res+1);
                            z_chunk_avg += *(res+2);
                        } else {
                            invalid_packets++;
                            if (*(res + 3) == 0) {
                                printf("        Ignoring packet. X: %d. Previous valid packet's X: %d. %d > 25.\n",*res,prev_valid_packet[0],abs(*res-prev_valid_packet[0]));
                            }
                            if (*(res + 3) == 1) {
                                printf("        Ignoring packet. Y: %d. Previous valid packet's Y: %d. %d > 25.\n",*(res+1),prev_valid_packet[1],abs(*(res+1)-prev_valid_packet[1]));
                            } 
                            if (*(res + 3) == 2) {
                                printf("        Ignoring packet. Z: %d. Previous valid packet's Z: %d. %d > 25.\n",*(res+2),prev_valid_packet[2],abs(*(res+2)-prev_valid_packet[2]));
                            }                             
                        }                            

                    } else {
                        printf("Packet not valid. Checksum incorrect.\n");
                    }
                }
                if (packet-invalid_packets > 0) {
                    printf("    Chunk Average X: %.2f, Average Y: %.2f, Average Z: %.2f\n\n", x_chunk_avg/(packet-invalid_packets), y_chunk_avg/(packet-invalid_packets), z_chunk_avg/(packet-invalid_packets));
                } else {
                    printf("No valid packets were found for this chunk.\n");
                }
                fseek(file,current_delim+5,SEEK_CUR);
            } 
        //Increment chunk counter and current position in array to read from.    
        current_delim = i+4;
        delim_counter++;
        }

    }

    fclose(file);
    return 0;

}

int check_delims(char d1[], char d2[], char d3[]){
    //Function to check command line arguments are valid delimiters.
    char* delims[3] = {d1,d2,d3};
    
    //Loop to check all delims for hex string formatting as well as containing valid hex digits.
    for (int i = 0; i < 3; i++) {
        char* delim = delims[i];

        if (delim[0] != '0') {
            return 0;
        }

        if (delim[1] != 'x') {
            return 0;
        }

        if (isxdigit(delim[2]) == 0 || isxdigit(delim[3]) == 0) {
            return 0;
        }

    }
    int delims_int[4];

    //Use parity byte function to confirm checksum is correct.
    for (int i = 0; i < 3; i++){
        delims_int[i] = (int)strtol(delims[i], NULL, 0);
        printf("Delimiter byte %d is: %d\n", i, delims_int[i]);
    }

    delims_int[3] = 0;

    // Calculate parity byte
    printf("Checksum is: %d\n\n", calc_parity_byte(delims_int));

    return 1;
}

int calc_parity_byte(int delims[4]) {
    // Function adapted to take in 4 argument array so that same parity byte calculation can be used for delimiters as well as packets.

    // Define result binary array as well as the delimiter binary 2d array
    int parity_byte[8];
    int delims_bin[4][8];

    //Convert to binary
    for (int i = 0; i < 4; i++){
        int j = 0;
        while (j < 8) {
            delims_bin[i][j] = delims[i] % 2;
            delims[i] = delims[i] / 2;
            j++;
        }
        //Reverse array
        int len = 8;
        int temp;
        for (j = 0; j < len/2; j++){
            temp = delims_bin[i][j];
            delims_bin[i][j] = delims_bin[i][len-j-1];
            delims_bin[i][len-j-1] = temp;
        }
        
        //Print bins
        // for (int k = 0; k < 8; k++){
        //     printf("%d ", delims_bin[i][k]);
        // }
        // printf("\n");
        
    }
    //Calculating parity byte based on binary conversion of delims
    for (int i = 0; i < 8; i++){
        parity_byte[i] = (delims_bin[0][i] + delims_bin[1][i] + delims_bin[2][i] + delims_bin[3][i]) % 2;
        //printf("%d", parity_byte[i]);
    }
    //printf("\n");
    //Convert parity byte to int
    int parity_byte_num = 0;
    for (int i = 7; i >= 0; i--){
        parity_byte_num += (1 << i)*(parity_byte[7-i]);
        
    }

    return parity_byte_num;
}


int * check_valid_packet_and_perform_swizzle(int curr_packet[4], int current_packet_index, int swizzle, int last_valid[3]) {
    // Define return result array
    static int result[4];

    //Operations based on swizzle
    switch(swizzle)
    {
   case 1:
        result[0] = curr_packet[0];
        result[1] = curr_packet[1];
        result[2] = curr_packet[2];
        printf("        Swizzle: XYZ\n");
        break;
    case 2:
        result[0] = curr_packet[0];
        result[1] = curr_packet[2];
        result[2] = curr_packet[1];
        printf("        Swizzle: XZY\n");
        break;
    case 3:
        result[0] = curr_packet[1];
        result[1] = curr_packet[0];
        result[2] = curr_packet[2];
        printf("        Swizzle: YXZ\n");
        break;
    case 4:
        result[0] = curr_packet[2];
        result[1] = curr_packet[0];
        result[2] = curr_packet[1];
        printf("        Swizzle: YZX\n");
        break;
    case 5:
        result[0] = curr_packet[1];
        result[1] = curr_packet[2];
        result[2] = curr_packet[0];
        printf("        Swizzle: ZXY\n");
        break;
    case 6:
        result[0] = curr_packet[2];
        result[1] = curr_packet[1];
        result[2] = curr_packet[0];
        printf("        Swizzle: ZYX\n");
        break;
    
    default:
        printf("Error. Swizzle not between 1-6\n");
    }

    //Check whether move is to be ignored - returns an indicator as to which coordinate fails test in 3rd element of return array
    if(last_valid[0] == -1000){
        result[3] = 3;
    } else if (abs(last_valid[0] - result[0]) > 25){
        result[3] = 0;
    } else if (abs(last_valid[1] - result[1]) > 25) {
        result[3] = 1;
    } else if(abs(last_valid[2] - result[2]) > 25 ){
        result[3] = 2;    
    } else {
        result[3] = 3;
    }

    return result;
}