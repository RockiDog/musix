/*********************************************************************
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                   Version 2, December 2004
 *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 *********************************************************************/

#ifndef MUSIX_SRC_CORE_CORE_H_
#define MUSIX_SRC_CORE_CORE_H_

#include <curl/curl.h>

#include <string>
#include <vector>

namespace musix {

/* The core & utility functions */
namespace utils {

std::vector<unsigned char> byte_array(const std::string& str);
std::vector<unsigned char> md5_encode(const std::vector<unsigned char>& in_str);
std::vector<unsigned char> base64_encode(const std::vector<unsigned char>& in_str);

/* Get the encrypted song id which is used in searching */
std::string encrypt_song_id(const std::string& song_id);

}

class Core {
 public:
  enum SearchType {
    SONG   = 1,
    ALBUM  = 10,
    ARTIST = 100,
    LIST   = 1000,
    USER   = 1002
  };

  static const char* SEARCH_URL;
  static const char* SEARCH_COOKIES;
  static const char* DOWNLOAD_URL_PREFIX;

  static Core* Instance();
  static void Release();

  std::string SearchAny(const std::string& keyword, int offset, int limit, SearchType type);
  void DownloadSong(const std::string& song_dfs_id, const std::string& filename);

 private:
  CURL* search_handle_;
  CURL* download_handle_;
  static Core* instance_;

  Core();
  ~Core();
  Core(const Core&);
  void operator=(const Core&);
};

}

#endif
