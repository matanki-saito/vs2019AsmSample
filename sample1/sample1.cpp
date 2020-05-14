// sample1.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "sample1.h"

using namespace std;

extern "C" {
	int geso();
}

int main()
{
	int result = geso();

	cout << "asm return : " << result << endl;

	return 0;
}
