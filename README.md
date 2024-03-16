Name: Daniel Chorev
unikey: dcho3009
 
File operations explanation:
My program firstly seeks to end the end of the file to find the file size.
Then, I use a loop for the size of the file and read in 1 byte at a time for 4 bytes into an array.
I compare these 4 bytes to the delimiters and the checksum, if it is a match, I store the offset in the file of this delimiter, seek back to the last delimiter with the file pointer and process the chunk data between those two offsets.
To process the chunk i begin at the end of the delimiter and read in 5 bytes, 1 at a time. This becomes a packet. I then check its validity and perform the required swizzle operation before moving 5 bytes forward in the file and repeating. I continue this for the number of packets in the chunk as was calculated by the difference in byte offset between the two found delimiters in the file.
I then seek forward 4 bytes ahead of the delimiter that was found and begin the delimiter search process again as described above. This continues until the end of the file. The program will also store the final character in the file to make sure that even if there is no delimiter at the end of the file, the chunk of data until the end of the file is still processed.