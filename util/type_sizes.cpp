#include <iostream>
using namespace std;

int main()
{
	cout << "double: " << sizeof(double) << " bytes" << endl;
	cout << "float: " << sizeof(float) << " bytes" << endl;
	cout << "char: " << sizeof(char) << " bytes" << endl;
	cout << "short: " << sizeof(short) << " bytes" << endl;
	cout << "int: " << sizeof(int) << " bytes" << endl;
	cout << "long: " << sizeof(long) << " bytes" << endl;
	cout << "void*: " << sizeof(void*) << " bytes" << endl;
	return 0;
}
