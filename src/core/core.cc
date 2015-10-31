#include "../core/core.h"

#include <curl/curl.h>

#include <cmath>
#include <cstring>
#include <sstream>
#include <string>

#include <iostream>

using namespace std;
using namespace musix;
using namespace musix::utils;

namespace {

const char* APPVER = "1.9.2";
const char* SEARCH_URL = "http://music.163.com/api/search/pc";

unsigned int left_rotate(unsigned int num, int r) {
  unsigned int temp = num;
  num <<= r;
  temp >>= 32 - r;
  return num + temp;
}

size_t receive_json(char* ptr, size_t size, size_t nmemb, void* userdata) {
  ostringstream& ostream = *static_cast<ostringstream*>(userdata);
  ostream.write(ptr, size * nmemb);
  return size * nmemb;
}

}

vector<unsigned char> byte_array(const string& str) {
  vector<unsigned char> arr;
  for (string::const_iterator it = str.begin(); it != str.end() - 1; ++it)
    arr.push_back(*it);
  return arr;
}

vector<unsigned char> musix::utils::md5_encode(const vector<unsigned char>& in_arr) {

  /* Complement */
  unsigned int len = in_arr.size();
  unsigned long bit_width = len * 8;
  unsigned char* byte_array = 0;
  unsigned int chunk_n = bit_width / 512 + (bit_width % 512 < 448 ? 1 : 2);
  unsigned int byte_n = chunk_n * 64;
  byte_array = new unsigned char[byte_n];
  memset(byte_array, 0, sizeof(unsigned char) * byte_n);
  for (vector<unsigned char>::const_iterator it = in_arr.begin(); it != in_arr.end(); ++it)
    byte_array[it - in_arr.begin()] = *it;
  byte_array[len] = 0x80;
  for (int i = 8; i > 0; --i)
    byte_array[byte_n - i] = ((unsigned char*)&bit_width)[8 - i];

  unsigned int h0 = 0x67452301;
  unsigned int h1 = 0xEFCDAB89;
  unsigned int h2 = 0x98BADCFE;
  unsigned int h3 = 0x10325476;

  int r[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
  };

  unsigned int k[64];
  for (int i = 0; i < 64; ++i)
    k[i] = (unsigned int)(abs(sin(i + 1)) * 0x100000000);

  for (int i = 0; i < chunk_n; ++i) {
    unsigned int* chunk = (unsigned int*)(byte_array + i * 64);
    
    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;
    
    unsigned int f, g;
    for (int j = 0; j < 64; ++j) {
      if (j >= 0 && j < 16) {
        f = (b & c) | ((~b) & d);
        g = j;
      } else if (j >= 16 && j < 32) {
        f = (b & d) | (c & (~d));
        g = (j * 5 + 1) % 16;
      } else if (j >= 32 && j < 48) {
        f = b ^ c ^ d;
        g = (j * 3 + 5) % 16;
      } else if (j >= 48 && j < 64) {
        f = c ^ (b | (~d));
        g = j * 7 % 16;
      }
    
      unsigned int temp = d;
      d = c;
      c = b;
      b += left_rotate(a + f + chunk[g] + k[j], r[j]);
      a = temp;
    }
    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
  }

  /* Digest */
  vector<unsigned char> out_arr(16);
  for (int i = 0; i < 4; ++i) {
    out_arr[i + 0] = h0 << (3 - i) * 8 >> 24;
    out_arr[i + 4] = h1 << (3 - i) * 8 >> 24;
    out_arr[i + 8] = h2 << (3 - i) * 8 >> 24;
    out_arr[i + 12] = h3 << (3 - i) * 8 >> 24;
  }
  delete [] byte_array;
  return out_arr;
}

vector<unsigned char> musix::utils::base64_encode(const vector<unsigned char>& in_arr) {
  unsigned int len = in_arr.size();
  unsigned int byte_n = len + (len % 3 == 0 ? 0 : (len % 3 == 2 ? 1 : 2));
  unsigned char* byte_array = new unsigned char[byte_n];
  memset(byte_array, 0, sizeof(unsigned char) * byte_n);
  for (vector<unsigned char>::const_iterator it = in_arr.begin(); it != in_arr.end(); ++it)
    byte_array[it - in_arr.begin()] = *it;

  const char code_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\n";
  unsigned int out_len = byte_n * 8 / 6 - (len % 3 == 0 ? 0 : (len % 3 == 2 ? 1 : 2));
  out_len += out_len / 76;
  out_len += (len % 3 == 0 ? 0 : (len % 3 == 2 ? 1 : 2));
  vector<unsigned char> out_arr;

  int offset = 0;
  for (int i = 0; i < out_len; ++i) {
    if (i % 77 == 76) {
      out_arr.push_back(65);
    } else if (offset < len * 8) {
      char v = 0;
      v += (unsigned char)(byte_array[offset / 8] << (offset % 8)) >> 2;
      if (offset % 8 > 2)
        v += (unsigned char)(byte_array[offset / 8 + 1]) >> (10 - offset % 8);
      out_arr.push_back(v);
      offset += 6;
    } else {
      out_arr.push_back(64);
    }
  }

  for (vector<unsigned char>::iterator it = out_arr.begin(); it != out_arr.end(); ++it) {
    int index = *it;
    *it = code_map[index];
  }
  delete [] byte_array;
  return out_arr;
}

string musix::utils::encrypt_song_id(const string& song_id) {
  char key[] = "3go8&$8*3*3h0k(2)2";
  unsigned int key_len = strlen(key);
  unsigned int id_len = song_id.length();
  vector<unsigned char> id_byte(id_len);
  for (int i = 0; i < id_len; ++i)
    id_byte[i] = song_id[i] ^ key[i % key_len];

  vector<unsigned char> encrypted = base64_encode((md5_encode(id_byte)));
  char* tmp_str = new char[encrypted.size() + 1];
  tmp_str[encrypted.size()] = 0;
  for (vector<unsigned char>::iterator it = encrypted.begin(); it != encrypted.end(); ++it) {
    if (*it == '/')
      tmp_str[it - encrypted.begin()] = '_';
    else if (*it == '+')
      tmp_str[it - encrypted.begin()] = '-';
    else
      tmp_str[it - encrypted.begin()] = *it;
  }
  string out_str(tmp_str);
  delete [] tmp_str;
  return out_str;
}


Core* Core::instance_ = nullptr;
const char* Core::SEARCH_URL = "http://music.163.com/api/search/pc";
const char* Core::SEARCH_COOKIES =  "appver=1.9.2;os=pc";

Core::Core() : search_handle_(nullptr) {
  curl_global_init(CURL_GLOBAL_ALL);
}

Core::~Core() {
  if (search_handle_ != nullptr)
    curl_easy_cleanup(search_handle_);
  curl_global_cleanup();
}

Core* Core::Instance() {
  if (instance_ == nullptr)
    instance_ = new Core();
  return instance_;
}

void Core::Release() {
  if (instance_ != nullptr)
    delete instance_;
  instance_ = nullptr;
}

string Core::SearchAny(const string& keyword, int type, int offset, int limit) {

  /* Initialization */
  if (search_handle_ == nullptr) {
    search_handle_ = curl_easy_init();
    curl_easy_setopt(search_handle_, CURLOPT_URL, SEARCH_URL);
  }

  ostringstream post_data_stream;
  ostringstream received_json;
  post_data_stream << "s="      << keyword << "&"
                   << "type="   << type    << "&"
                   << "offset=" << offset  << "&"
                   << "limit="  << limit;
  string post_data = post_data_stream.str();
  curl_easy_setopt(search_handle_, CURLOPT_POSTFIELDS, post_data.c_str());
  curl_easy_setopt(search_handle_, CURLOPT_COOKIE, SEARCH_COOKIES);
  curl_easy_setopt(search_handle_, CURLOPT_WRITEDATA, &received_json);
  curl_easy_setopt(search_handle_, CURLOPT_WRITEFUNCTION, receive_json);

  curl_easy_perform(search_handle_);
  return received_json.str();
}
