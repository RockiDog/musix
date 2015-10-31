#include "core.h"

#include <iostream>
#include <string>

using namespace std;
using namespace musix;

int main() {
  string json = Core::Instance()->search_anything("突然的自我", 1, 0, 1);
  cout << json << endl;
  Core::Release();
  return 0;
}
