#include "../test.h"

cube_t applymoves(cube_t, char *);

int main(void) {
	char movestr[STRLENMAX], cubestr[STRLENMAX];
	cube_t cube;

	fgets(movestr, STRLENMAX, stdin);
	fgets(cubestr, STRLENMAX, stdin);
	cube = readcube("H48", cubestr);

	cube = applymoves(cube, movestr);

	if (cube_error(cube)) {
		printf("Error moving cube\n");
	} else if (!cube_solvable(cube)) {
		printf("Moved cube is not solvable\n");
	} else {
		writecube("H48", cube, cubestr);
		printf("%s\n", cubestr);
	}

	return 0;
}
