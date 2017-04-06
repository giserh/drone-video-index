#include "aerialfov.h"
#include <stdio.h>


using namespace std;



/***
 * Transfer aerial-FOV object file from txt format to binary format.
 *
 * @para[in] txt object file name
 * @para[in] binary object file name
 *
 * Output: the binary file
 */
void txt2binary_transfer(char* txt_object_filename, char* binary_object_filename)
{
	FILE* txt_object_file = fopen(txt_object_filename, "r+");
	if (txt_object_file == NULL) {
		printf("Couldn't open %s\n", txt_object_filename);
		return;
	}
	
	FILE* binary_object_file = fopen(binary_object_filename, "rb+");
	if (binary_object_file == NULL) {
		printf("Couldn't open %s\n", binary_object_filename);
		return;
	}
	
	AerialFOV fov;
	while(!feof(txt_object_file))
	{
		fscanf(txt_object_file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
		                         &fov.quadrilateral[0],
								 &fov.quadrilateral[1],
								 &fov.quadrilateral[2],
								 &fov.quadrilateral[3],
								 &fov.quadrilateral[4],
								 &fov.quadrilateral[5],
								 &fov.quadrilateral[6],
								 &fov.quadrilateral[7],
								 &fov.azimuth,
								 &fov.timestamp);
		/*printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		                         fov.quadrilateral[0],
								 fov.quadrilateral[1],
								 fov.quadrilateral[2],
								 fov.quadrilateral[3],
								 fov.quadrilateral[4],
								 fov.quadrilateral[5],
								 fov.quadrilateral[6],
								 fov.quadrilateral[7],
								 fov.azimuth,
								 fov.timestamp);*/
		fwrite(&fov, sizeof(AerialFOV), 1, binary_object_file);
	}
	fclose(txt_object_file);
	fclose(binary_object_file);
}




/***
 * Read an aerial-FOV object from the binary object file.
 *
 * @para[in] binary object file name
 * @para[in] objectid: the object id of the aerial-FOV object to be read.
 *
 * Output: print the aerial-FOV object.
 */
void read_binary_object_file(char* binary_object_filename, int objectid)
{	
	FILE* binary_object_file = fopen(binary_object_filename, "rb+");
	if (binary_object_file == NULL) {
		printf("Couldn't open %s\n", binary_object_filename);
		return;
	}
	
	AerialFOV fov;
	fseek(binary_object_file, (objectid-1)*sizeof(AerialFOV), 0);
	fread(&fov, sizeof(AerialFOV), 1, binary_object_file);
	printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
							 fov.quadrilateral[0],
							 fov.quadrilateral[1],
							 fov.quadrilateral[2],
							 fov.quadrilateral[3],
							 fov.quadrilateral[4],
							 fov.quadrilateral[5],
							 fov.quadrilateral[6],
							 fov.quadrilateral[7],
							 fov.azimuth,
							 fov.timestamp);
								 
	/*while(!feof(binary_object_file))
	{
		fread(&fov, sizeof(AerialFOV), 1, binary_object_file);
		printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		                         fov.quadrilateral[0],
								 fov.quadrilateral[1],
								 fov.quadrilateral[2],
								 fov.quadrilateral[3],
								 fov.quadrilateral[4],
								 fov.quadrilateral[5],
								 fov.quadrilateral[6],
								 fov.quadrilateral[7],
								 fov.azimuth,
								 fov.timestamp);
	}*/
	fclose(binary_object_file);
}



int main(int argc, char* argv[])
{
	char* txt_object_filename = argv[1];
	char* binary_object_filename = argv[2];
	
	std::string func(argv[3]);
	if(func=="transfer") txt2binary_transfer(txt_object_filename, binary_object_filename);
	if(func=="read") 
	{
		int objectid = stoi(argv[4]);
		read_binary_object_file(binary_object_filename, objectid);
	}
	return 1;
}