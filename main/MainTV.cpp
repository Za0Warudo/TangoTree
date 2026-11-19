/**
 * @file MainTV.cpp
 * @version 1.0
 * @date 2026-11-15
 *
 * The main program of the Tango Tree visualization. It uses the TangoVisualization class, a copy of the Tango Tree that records every step of a search.
 *
 * To keep the picture readable, the tree can only have 15, 31 or 63 keys. These are the sizes of a full reference tree with 4, 5 or 6 levels.
 *
 * This file is built in two ways:
 *
 *  - As a normal program (MainTV). It reads n and then the keys to search for, and prints the steps of each search as text.
 *  - With Emscripten, as a WebAssembly module. It then has no main function. Instead, it exports the functions tvInit, tvSearch and tvState, which are called
 *    by the page in the visualization directory. See visualization/CMakeLists.txt.
 *
 * How to use the normal program:
 * First type n (15, 31 or 63). Then type any number of keys to search for. Add the option --json to print the frames as JSON instead, one line per search, in
 * the same format the page reads.
 *
 * * Example:
 * * input:
 *      15
 *      5
 * * output:
 *      A new tango tree with the keys 1 to 15. Every key starts as its own auxiliary tree.
 *      search 5 (79 steps): True
 *        1. [search 5] Search for 5. Start at 8, the root of the tango tree.
 *        2. [search 5] 5 is smaller than 8: go left.
 *        ...
 *        79. [search 5] Done. 5 is now in the top auxiliary tree, together with every node that is above it in the reference tree.
 */

// includes.
#include "TangoVisualization.h"
#include <iostream>
#include <memory>
#include <string>

/**
 * @brief Checks if the visualization supports a tree with n keys.
 */
static bool isSupportedSize(int n) { return n == 15 || n == 31 || n == 63; }

#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>

static std::unique_ptr<TangoVisualization> tree;     // The tree shown in the page.
static std::string json;                             // The text returned to the page. It must stay alive until the page reads it.

extern "C" {

/**
 * @brief Builds a new tree with the keys 1 to n, replacing the current one.
 * @return 1 if the tree was built, or 0 if n is not 15, 31 or 63.
 */
EMSCRIPTEN_KEEPALIVE int tvInit(int n) {
  if (!isSupportedSize(n))
    return 0;
  tree = std::make_unique<TangoVisualization>(n);
  return 1;
}

/**
 * @brief Searches for the given key and returns the frames of the search as JSON. Returns an empty object if there is no tree yet.
 */
EMSCRIPTEN_KEEPALIVE const char *tvSearch(int key) {
  if (!tree)
    return "{}";
  tree->search(key);
  json = tree->toJson();
  return json.c_str();
}

/**
 * @brief Returns the frames of the last search as JSON, or the single frame of a newly built tree. Returns an empty object if there is no tree yet.
 */
EMSCRIPTEN_KEEPALIVE const char *tvState() {
  if (!tree)
    return "{}";
  json = tree->toJson();
  return json.c_str();
}
}

#else

int main(int argc, char **argv) {
  const bool asJson = argc > 1 && std::string(argv[1]) == "--json";

  int n, key;
  if (!(std::cin >> n) || !isSupportedSize(n)) {
    std::cerr << "The visualization only supports trees with 15, 31 or 63 keys." << std::endl;
    return 1;
  }
  TangoVisualization t(n);

  if (asJson)
    std::cout << t.toJson() << std::endl;
  else
    std::cout << t.getFrames().front().text << std::endl;

  while (std::cin >> key) {
    const bool found = t.contains(key);
    if (asJson) {
      std::cout << t.toJson() << std::endl;
      continue;
    }

    const std::vector<TraceFrame> &frames = t.getFrames();
    std::cout << "search " << key << " (" << frames.size() << " steps): " << (found ? "True" : "False") << std::endl;
    for (size_t i = 0; i < frames.size(); ++i) {
      std::cout << "  " << i + 1 << ". [";
      for (size_t j = 0; j < frames[i].context.size(); ++j)
        std::cout << (j > 0 ? " > " : "") << frames[i].context[j];
      std::cout << "] " << frames[i].text << std::endl;
    }
  }
  return 0;
}

#endif
