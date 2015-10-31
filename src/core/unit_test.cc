#include "core.h"

#include <iostream>
#include <string>

using namespace std;
using namespace musix;

int main() {
  string mp3_filename = "突然的自我.mp3";
  Core::Instance()->DownloadSong("3350211930003282", mp3_filename);
  Core::Release();
  return 0;
}
