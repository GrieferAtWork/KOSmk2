/* Compile with 'tcc main.c' */

#include <stdio.h>

int main(int argc, char **argv) {
	printf("The .exe that this application compiled to can be executed under windows.\n");
	printf("But it can also run natively within KOS!!!\n");
	printf("HINT: It doesn't work if the compiler is too complicated (like msvc) "
	       "or doesn't link against msvcrtXXX.dll.\n");
	printf("    - Try using tcc. It work very well.\n");
	printf("Also worth mentioning is the fact that the 'printf()' function %s\n",
	       "That I keep on calling here is located with 'libc.so'. - Yes: An ELF library");
	printf("So as you can see, KOS manages to implement TRUE cross-platform execution, %q %s\n",
	       "\nLiterally\n","combining two things that were never meant to be combined.");
	printf("\n\n\n");
	printf("If you're wondering how far this goes, take a look at '__USE_DOS' sections in KOS's system headers\n");
	printf("You'll find that this goes _very_ far, including binary compatibility with MSVC's (DOS's) public data structures. such as 'stat', 'finddata' and 'FILE'\n");
	printf("PS: The text above uses some printf() extensions provided by KOS, such as '%%q'.\n");
	printf("    When being executed on windows, DOS won't be able to interpret them...\n");
	return 0;
}

