#include <iostream>

int* fun(int* a) {
	*a = *a + 1;
	return a;
}


int main() {
	int a = 1;

	fun(fun(fun(&a)));
	/*
	

		004119C0  mov         dword ptr [a],1
		004119C7  lea         eax,[a]
		004119CA  push        eax
		004119CB  call        fun (041125Dh)
		004119D0  add         esp,4
		004119D3  push        eax
		004119D4  call        fun (041125Dh)
		004119D9  add         esp,4

	*/

	printf("%d\n", a);
	return 0;
}